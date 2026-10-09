#include "CProjectile.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CVidMode.h"
#include "CGameEffect.h"
#include "CGameAnimationTypeEffect.h"
#include "IcewindCGameAnimationTypeEffect.h"
#include "CGameArea.h"
#include "CGameContainer.h"
#include "CGameDoor.h"
#include "CGameTrigger.h"
#include "CGameFireball3d.h"
#include "CMessage.h"
#include "CGameObjectArray.h"
#include "CParticle.h"
#include "CGameSprite.h"
#include "CGameTemporal.h"
#include "CInfinity.h"
#include "CInfGame.h"
#include "CPathSearch.h"
#include "CResBitmap.h"
#include "CUtil.h"
#include "CTimerWorld.h"
#include "IcewindMisc.h"

// 0x84F06C. Data-segment gather radius the image initialises to 100 and never
// writes; DecodeProjectile case 0x118 is its only reader in the whole binary.
static SHORT g_nSpellHitDefaultRange = 100;

// 0x52A640
void CProjectile::GetStart(LONG source, CPoint& pt, BYTE facingOffset)
{
    CGameObject* pSource;
    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(source,
        CGameObjectArray::THREAD_ASYNCH, &pSource, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    BYTE type = pSource->GetObjectType();
    if (type == CGameObject::TYPE_NONE || type == CGameObject::TYPE_AIBASE
        || type == CGameObject::TYPE_SOUND || type == CGameObject::TYPE_SPAWNING
        || type == CGameObject::TYPE_STATIC || type == CGameObject::TYPE_SPRITE
        || type == CGameObject::TYPE_OBJECT_MARKER || type == CGameObject::TYPE_TEXT) {
        pt = pSource->GetPos();
    } else if (type == CGameObject::TYPE_CONTAINER) {
        pt = static_cast<CGameContainer*>(pSource)->m_posTrapOrigin;
    } else if (type == CGameObject::TYPE_DOOR) {
        CGameDoor* pDoor = static_cast<CGameDoor*>(pSource);
        pt = CPoint(pDoor->m_posXTrapOrigin, pDoor->m_posYTrapOrigin);
    } else if (type == CGameObject::TYPE_TRIGGER) {
        pt = static_cast<CGameTrigger*>(pSource)->m_posTrapOrigin;
    } else {
        UTIL_ASSERT(FALSE);
    }

    if (facingOffset && pSource->GetObjectType() == CGameObject::TYPE_SPRITE) {
        CGameSprite* pSprite = static_cast<CGameSprite*>(pSource);
        SHORT direction = pSprite->GetDirection();
        switch (direction) {
        case 0:
        case 1:
            pt.y += pSprite->GetAnimation()->GetEllipseRect().bottom >> 1;
            break;
        case 4:
        case 5:
            pt.x -= 2 * pSprite->GetAnimation()->GetEllipseRect().right;
            ++pt.y;
            break;
        case 8:
        case 9:
            pt.y -= 2 * pSprite->GetAnimation()->GetEllipseRect().bottom;
            break;
        case 12:
        case 13:
            pt.x += 2 * pSprite->GetAnimation()->GetEllipseRect().right;
            ++pt.y;
            break;
        case 2:
        case 3:
        case 6:
        case 7:
        case 10:
        case 11:
        case 14:
        case 15: {
            const CRect& ellipse = pSprite->GetAnimation()->GetEllipseRect();
            SHORT xSquared = static_cast<SHORT>(static_cast<SHORT>(ellipse.right) * static_cast<SHORT>(ellipse.right));
            SHORT ySquared = static_cast<SHORT>(static_cast<SHORT>(ellipse.bottom) * static_cast<SHORT>(ellipse.bottom));
            SHORT diagonal = static_cast<SHORT>(sqrt(static_cast<double>(
                (xSquared * ySquared) / (xSquared + ySquared))));
            pt.x += (direction < 8 ? -2 : 2) * diagonal;
            pt.y += (direction < 4 || direction >= 14 ? 2 : -2) * diagonal;
            break;
        }
        default:
            UTIL_ASSERT(FALSE);
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(source,
        CGameObjectArray::THREAD_ASYNCH, INFINITE);
}

// 0x51E9B0
CProjectile::~CProjectile()
{
    POSITION pos = m_effectList.GetHeadPosition();
    while (pos != NULL) {
        delete m_effectList.GetNext(pos);
    }
    m_effectList.RemoveAll();
}

// 0x536F40
BOOLEAN CProjectile::CompressTime(DWORD deltaTime)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
    // __LINE__: 10139
    UTIL_ASSERT(deltaTime > static_cast<DWORD>(CTimerWorld::TIMESCALE_MSEC_PER_SEC)
        * CTimerWorld::TIMESCALE_SEC_PER_MIN);

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        delete this;
    }
    return FALSE;
}

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

    if (m_nTargetId != CGameObjectArray::INVALID_INDEX) {
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_nTargetId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pProjectile),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            // Direct base call in the binary at 0x52A118.
            pProjectile->CProjectile::RemoveSelf();

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_nTargetId,
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

// Dispatch an impact's ordered effect messages together (subtype 105).
// 0x52A1A0
void CProjectile::DeliverEffects()
{
    CGameObject* pTarget;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH, &pTarget, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    if ((pTarget->GetObjectType() & CGameObject::TYPE_AIBASE) == 0) {
        ClearEffects();
    } else if (pTarget->GetObjectType() == CGameObject::TYPE_SPRITE
        && IsTargetImmune(static_cast<CGameSprite*>(pTarget))) {
        ClearEffects();
        CGameEffect::FeedBackImmuneToResource(static_cast<CGameSprite*>(pTarget), m_casterResRef);
    } else {
        // The impact owns one message containing the ordered effect messages.
        // Its constructor copies nodes; it takes ownership of their messages.
        CMessage105::Node head = { &head, &head, NULL };
        POSITION pos = m_effectList.GetHeadPosition();
        while (pos != NULL) {
            CMessage105::Node* node = new CMessage105::Node;
            node->m_pNext = &head;
            node->m_pPrev = head.m_pPrev;
            node->m_pMessage = new CMessageAddEffect(m_effectList.GetNext(pos), m_sourceId, m_targetId);
            head.m_pPrev->m_pNext = node;
            head.m_pPrev = node;
        }
        m_effectList.RemoveAll();
        CMessage105* message = new CMessage105(&head, m_sourceId, m_targetId);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH, INFINITE);
        while (head.m_pNext != &head) {
            CMessage105::Node* node = head.m_pNext;
            head.m_pNext = node->m_pNext;
            delete node;
        }
        return;
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
        CGameObjectArray::THREAD_ASYNCH, INFINITE);
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

// 0x536FC0
// TRUE when the target is immune to this projectile: the projectile type is
// on the target's projectile-immunity list, or m_nSpellLevel (when <= 8)
// indexes a set slot of the spell-level immunity table (the table
// CGameEffectImmunityToSpellLevel writes).
BOOL CProjectile::IsTargetImmune(CGameSprite* pSprite)
{
    if (pSprite->GetDerivedStats()->m_cImmunitiesProjectile.OnList(m_projectileType) == 0
        && ((m_nSpellLevel & 0xFF) > 8
            || pSprite->GetDerivedStats()->m_cImmunitiesSpellLevel.m_levels[m_nSpellLevel & 0xFF] == 0)) {
        return FALSE;
    }

    return TRUE;
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

// 0x529490
CProjectileInstant::CProjectileInstant()
{
    // Default-constructs the CProjectile base; the compiler stamps this class's
    // own vtable (0x84D8B8).  The only override is Fire (instant delivery).
}

// 0x529950 (slot 27).  Instant delivery -- no travel, no area registration.
// Records the launch ids/area, runs OnArrival (which calls DeliverEffects)
// straight away, then self-deletes.  Because the projectile was never added to
// the global object array, OnArrival's Delete() no-ops, so this trailing delete
// is the only one (single-shot).
void CProjectileInstant::Fire(CGameArea* pArea, LONG source, LONG target,
                              CPoint targetPos, LONG nHeight, SHORT nType)
{
    (void)targetPos;
    (void)nHeight;
    (void)nType;

    m_targetId = target;
    m_sourceId = source;
    m_pArea = pArea;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;

    OnArrival();
    delete this;
}

// 0x51EAF0
//
// Factory that maps a numeric projectile type to a concrete CProjectile
// subclass. The original dispatches ~327 hardcoded types (projectileType - 1
// indexes a 386-entry jump table) plus a generic path for types > 0x1000
// (handled by the school-overlay sub-factory at 0x560310). The casting-glow
// overlays (CProjectileBAM), the summon/spell-hit VFX (CProjectileSummonVFX) and
// the canonical travelling arrow (CProjectileArrow) are recovered here; the
// remaining hardcoded classes are left unimplemented rather than guessed.
CProjectile* CProjectile::DecodeProjectile(USHORT projectileType, CGameAIBase* pCaster, BYTE castDelay)
{
    IcewindCVisualEffect visualEffect;

    if (projectileType > 0x1000) {
        CProjectile* pSpellHit = CProjectileSummonVFX::DecodeSpellHitProjectile(
            projectileType - 0x1001, pCaster, FALSE);
        if (pSpellHit != NULL) {
            pSpellHit->m_projectileType = projectileType - 1;
        }
        return pSpellHit;
    }

    CProjectile* pProjectile = NULL;
    switch (projectileType) {
    case 0x1:
        // 0x51EB93: the "None" projectile -- CProjectileInstant delivers its
        // payload immediately (Fire -> OnArrival -> DeliverEffects, no travel)
        // then self-deletes.  Used by missileType=1 spells whose effects are
        // applied at cast (e.g. Call Lightning's opcode-449 Static Charge).
        // Previously fell through to the default plain CProjectile, whose Fire
        // is an empty stub -> the effects were silently dropped.
        pProjectile = new CProjectileInstant();
        break;

    case 0x2:
    case 0x5:
    case 0x6:
        // ARARROW -- the canonical travelling arrow.
        pProjectile = new CProjectileArrow();
        break;

    case 0xC:
    case 0xF:
    case 0x10:
        // 0x51F08A / 0x51F208 / 0x51F276: ordinary crossbow bolts.
        pProjectile = new CProjectileBolt();
        pProjectile->m_bHasHeight = TRUE;
        pProjectile->m_fireSoundRef = CResRef("");
        pProjectile->m_arrivalSoundRef = CResRef("");
        break;

    case 0x11:
    case 0x14:
    case 0x15:
        // 0x51F389 / 0x51F49D / 0x51F50B: ordinary sling stones.
        pProjectile = new CProjectileSlingStone();
        pProjectile->m_bHasHeight = TRUE;
        pProjectile->m_fireSoundRef = CResRef("");
        pProjectile->m_arrivalSoundRef = CResRef("");
        break;

    case 0x3: {
        // The exploding flame missile: flies its default SPFLMARR cell to the
        // target and runs the strike pass there, launch sound TRA_10.
        CProjectileExplodingFlame* pMissile = new CProjectileExplodingFlame();
        pMissile->m_bHasHeight = TRUE;
        pMissile->m_fireSoundRef = CResRef("TRA_10");
        pMissile->m_arrivalSoundRef = CResRef("");
        pProjectile = pMissile;
        break;
    }

    case 0x8:    // exploding thrown axe ("AXE")
    case 0xD:    // exploding crossbow bolt ("BOLT")
    case 0x12:   // exploding sling stone ("MAGICSTN")
    case 0x1C:   // exploding throwing dagger ("DAGGER")
    case 0x21:   // exploding dart ("DART")
    case 0x39: { // exploding spear ("SPEAR")
        // The exploding thrown-weapon missiles: the strike-pass leaf with its
        // cell swapped to the weapon BAM. Only the bolt has a launch sound.
        CProjectileExplodingWeapon* pMissile = new CProjectileExplodingWeapon();
        CResRef fireSoundRef("");
        switch (projectileType) {
        case 0x8: pMissile->SetVidCell(CResRef("AXE")); break;
        case 0xD:
            pMissile->SetVidCell(CResRef("BOLT"));
            fireSoundRef = "TRA_10";
            break;
        case 0x12: pMissile->SetVidCell(CResRef("MAGICSTN")); break;
        case 0x1C: pMissile->SetVidCell(CResRef("DAGGER")); break;
        case 0x21: pMissile->SetVidCell(CResRef("DART")); break;
        case 0x39: pMissile->SetVidCell(CResRef("SPEAR")); break;
        }
        pMissile->m_bHasHeight = TRUE;
        pMissile->m_fireSoundRef = fireSoundRef;
        pMissile->m_arrivalSoundRef = CResRef("");
        pProjectile = pMissile;
        break;
    }

    case 0x68:   // palette range 0x45
    case 0xCC: { // palette range 0x47
        // The palette-tinted exploding-missile variants: the strike-pass leaf
        // flying its SPFIREBL cell with the palette re-ranged and the trail
        // and explosion colour ranges stamped to match, launch sound TRA_06.
        // (The original hands the palette to the cell before re-ranging it.)
        CProjectileExplodingWeapon* pMissile = new CProjectileExplodingWeapon();
        BYTE rangeValue = projectileType == 0x68 ? 0x45 : 0x47;
        pMissile->m_pVidCell->SetPalette(pMissile->m_palette);
        pMissile->m_palette.SetRange(0, rangeValue, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
        memset(pMissile->m_trailColorRanges, rangeValue, sizeof(pMissile->m_trailColorRanges));
        pMissile->m_explodeColorRange = rangeValue;
        pMissile->m_bHasHeight = TRUE;
        pMissile->m_fireSoundRef = CResRef("TRA_06");
        pMissile->m_arrivalSoundRef = CResRef("");
        pProjectile = pMissile;
        break;
    }

    case 0x4F:
        // The invisible per-target strike bolt the exploding missiles fire.
        pProjectile = new CProjectileStrike();
        break;

    case 0x20:
    case 0x23:
    case 0x24: {
        // The thrown dart (standard dart items carry missile type 0x24),
        // silent launch.
        CProjectileDart* pDart = new CProjectileDart();
        pDart->m_bHasHeight = TRUE;
        pDart->m_fireSoundRef = CResRef("");
        pDart->m_arrivalSoundRef = CResRef("");
        pProjectile = pDart;
        break;
    }

    case 0x4: {
        // The flaming arrow: the flame-trailed leaf flying its default
        // SPFLMARR cell, launch sound TRA_24.
        CProjectileSPFLMARR* pMissile = new CProjectileSPFLMARR();
        pMissile->m_bHasHeight = TRUE;
        pMissile->m_fireSoundRef = CResRef("TRA_24");
        pMissile->m_arrivalSoundRef = CResRef("");
        pProjectile = pMissile;
        break;
    }

    case 0x9:    // thrown axe ("AXE")
    case 0xE:    // crossbow bolt ("BOLT")
    case 0x1D:   // throwing dagger ("DAGGER")
    case 0x22: { // dart ("DART")
        // The thrown/launched weapon missiles: the flame-trailed leaf with
        // its cell swapped to the weapon BAM, launch sound TRA_10.
        CProjectileSPFLMARR* pMissile = new CProjectileSPFLMARR();
        switch (projectileType) {
        case 0x9: pMissile->SetVidCell(CResRef("AXE")); break;
        case 0xE: pMissile->SetVidCell(CResRef("BOLT")); break;
        case 0x1D: pMissile->SetVidCell(CResRef("DAGGER")); break;
        case 0x22: pMissile->SetVidCell(CResRef("DART")); break;
        }
        pMissile->m_bHasHeight = TRUE;
        pMissile->m_fireSoundRef = CResRef("TRA_10");
        pMissile->m_arrivalSoundRef = CResRef("");
        pProjectile = pMissile;
        break;
    }

    case 0x66:   // palette range 0x45
    case 0x67:   // palette range 0x44
    case 0xBC: { // palette range 0x47
        // The palette-tinted weapon-missile variants: the flame-trailed leaf
        // flying its SPFLMARR cell with the palette re-ranged and the trail
        // colour ranges stamped to match, launch sound TRA_10. (The original
        // hands the palette to the cell before re-ranging it, in this order.)
        CProjectileSPFLMARR* pMissile = new CProjectileSPFLMARR();
        BYTE rangeValue = 0;
        switch (projectileType) {
        case 0x66: rangeValue = 0x45; break;
        case 0x67: rangeValue = 0x44; break;
        case 0xBC: rangeValue = 0x47; break;
        }
        pMissile->m_pVidCell->SetPalette(pMissile->m_palette);
        pMissile->m_palette.SetRange(0, rangeValue, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
        memset(pMissile->m_trailColorRanges, rangeValue, sizeof(pMissile->m_trailColorRanges));
        pMissile->m_bHasHeight = TRUE;
        pMissile->m_fireSoundRef = CResRef("TRA_10");
        pMissile->m_arrivalSoundRef = CResRef("");
        pProjectile = pMissile;
        break;
    }

    case 0xDA:
        // MMissiT -- Magic Missile homing sub-missile (spawned by the launcher).
        // The factory case stamps the projectile's launch ("whoosh") sound, which
        // CProjectileTravelling::Fire plays when the missile enters the area. The
        // launcher (CProjectileMagicMissileMulti) then clears it on every
        // sub-missile after the first, so a volley whooshes once. (The original
        // handler at 0x5252AD also sets m_bHasHeight = TRUE and clears
        // m_arrivalSoundRef -- inert while the launch-height path is stubbed.)
        pProjectile = new CProjectileMMissiT(0);
        pProjectile->m_fireSoundRef = CResRef("TRA_02");
        break;

    case 0x18: {
        // CHROMORB -- Chromatic Orb's travelling orb ("COrbT" BAM): the
        // copy-from-back tinted travelling VFX, launch sound TRA_23.
        IcewindCProjectileTravellingVFX* pOrb = new IcewindCProjectileTravellingVFX(CResRef("COrbT"));
        pOrb->m_visualEffect.SetCopyFromBack(TRUE);
        pOrb->m_fireSoundRef = CResRef("TRA_23");
        pOrb->m_bHasHeight = TRUE;
        pOrb->m_arrivalSoundRef = CResRef("");
        pProjectile = pOrb;
        break;
    }

    // The single-target spell-projectile family: each of these builds the
    // IcewindCProjectileTravellingVFX carrier directly (like case 0x18) and
    // stamps its own animation cell / velocity / facing / sound, so the spell's
    // effects ride the recovered travel -> OnArrival -> DeliverEffects path.
    // Previously every one fell to `default: new CProjectile()` (empty Fire),
    // silently dropping the payload.  Transcribed from the 0x528e7c jump-table
    // handlers (blocks 0x525a55 / 0x525f00 / 0x525fec / 0x526492 / 0x526a35 /
    // 0x527436 / 0x527c77); the ctor 0x578110 already seeds height + colours.
    case 0xFB: {
        // ICELANT -- Ice Lance's travelling ice bolt ("IcelanT" BAM): the
        // copy-from-back tinted travelling VFX, 16 directional sequences,
        // launch sound TRA_19.
        IcewindCProjectileTravellingVFX* pLance = new IcewindCProjectileTravellingVFX(CResRef("IcelanT"));
        pLance->m_visualEffect.SetCopyFromBack(TRUE);
        pLance->m_dirCount = 0x10;
        pLance->m_fireSoundRef = CResRef("TRA_19");
        pProjectile = pLance;
        break;
    }

    case 0x10C: {
        // DISINTT -- Disintegrate's travelling ray ("DisintT" BAM): the
        // copy-from-back tinted travelling VFX, 16 directional sequences,
        // launch sound TRA_20.
        IcewindCProjectileTravellingVFX* pRay = new IcewindCProjectileTravellingVFX(CResRef("DisintT"));
        pRay->m_visualEffect.SetCopyFromBack(TRUE);
        pRay->m_dirCount = 0x10;
        pRay->m_fireSoundRef = CResRef("TRA_20");
        pProjectile = pRay;
        break;
    }

    case 0x10F: {
        // OFSPHET -- Otiluke's Freezing Sphere travelling orb ("OFSpheT" BAM):
        // the copy-from-back tinted travelling VFX, 16 directional sequences.
        // The block writes no sound or height override (ctor defaults stand).
        IcewindCProjectileTravellingVFX* pSphere = new IcewindCProjectileTravellingVFX(CResRef("OFSpheT"));
        pSphere->m_visualEffect.SetCopyFromBack(TRUE);
        pSphere->m_dirCount = 0x10;
        pProjectile = pSphere;
        break;
    }

    case 0x11D: {
        // MSPORET -- the Myconid "Sprays Spores" travelling VFX ("MSporeT" BAM).
        // The factory first shows the "Sprays Spores" combat-feedback line
        // (strref 4386) over the caster, then builds the plain travelling VFX
        // with no member overrides (constructor defaults only).
        IcewindMisc::DisplayFeedbackMessage(static_cast<CGameSprite*>(pCaster), 4386, 0);
        pProjectile = new IcewindCProjectileTravellingVFX(CResRef("MSporeT"));
        break;
    }

    case 0x12A: {
        // ALANCET -- Alicorn Lance (SPPR216.SPL, strref 21421): the silver
        // alicorn-horn lance's travelling VFX ("ALanceT" BAM), a copy-from-back
        // tinted bolt flying at double the ctor's base velocity, 16 rotation
        // frames, north-facing fold off, launch sound TRA_57.
        IcewindCProjectileTravellingVFX* pLance = new IcewindCProjectileTravellingVFX(CResRef("ALanceT"));
        pLance->m_velocity = static_cast<SHORT>(pLance->m_velocity * 2);
        pLance->m_visualEffect.SetCopyFromBack(TRUE);
        pLance->m_dirCount = 0x10;
        pLance->m_bMirrorNorth = 0;
        pLance->m_fireSoundRef = CResRef("TRA_57");
        pProjectile = pLance;
        break;
    }

    case 0x13C: {
        // VSPHERT -- Vitriolic Sphere's travelling acid orb ("VSpherT" BAM):
        // the copy-from-back tinted travelling VFX flying at double the ctor's
        // base velocity, single (non-directional) sequence, height + area
        // height-offset on, launch sound TRA_60, no arrival sound.
        IcewindCProjectileTravellingVFX* pSphere = new IcewindCProjectileTravellingVFX(CResRef("VSpherT"));
        pSphere->m_visualEffect.SetCopyFromBack(TRUE);
        pSphere->m_velocity = static_cast<SHORT>(pSphere->m_velocity << 1);
        pSphere->m_dirCount = 1;
        pSphere->m_bHasHeight = TRUE;
        pSphere->m_useHeightOffset = 1;
        pSphere->m_fireSoundRef = CResRef("TRA_60");
        pSphere->m_arrivalSoundRef = CResRef("");
        pProjectile = pSphere;
        break;
    }

    case 0x158: {
        // MFMISST -- the travelling fire-missile VFX ("MFMissT" BAM): a
        // copy-from-back tinted bolt flying at double the ctor's base velocity,
        // height + area height-offset on.  No sound or facing overrides.
        IcewindCProjectileTravellingVFX* pMissile = new IcewindCProjectileTravellingVFX(CResRef("MFMissT"));
        pMissile->m_velocity = static_cast<SHORT>(pMissile->m_velocity * 2);
        pMissile->m_visualEffect.SetCopyFromBack(TRUE);
        pMissile->m_bHasHeight = TRUE;
        pMissile->m_useHeightOffset = 1;
        pProjectile = pMissile;
        break;
    }

    case 0x25: {
        // MAGICMIS -- the plain single Magic Missile bolt (SPMAGMIS BAM, no
        // sparkle trail), launch whoosh TRA_02 like the launcher's
        // sub-missiles.
        CProjectileSparkle* pSparkle = new CProjectileSparkle(CVidPalette::TYPE_RESOURCE);
        pSparkle->m_fireSoundRef = CResRef("TRA_02");
        pSparkle->m_bHasHeight = TRUE;
        pSparkle->m_arrivalSoundRef = CResRef("");
        pProjectile = pSparkle;
        break;
    }

    case 0x2F:   // SPARKLBL blue
    case 0x30:   // SPARKLGO gold
    case 0x31:   // SPARKLPU purple
    case 0x32:   // SPARKLIC ice
    case 0x33:   // SPARKLST stone
    case 0x34:   // SPARKLBK black
    case 0x35:   // SPARKLCH chromatic
    case 0x36:   // SPARKLRE red
    case 0x37:   // SPARKLGR green
    case 0xB8:   // SPARKLMA magenta
    case 0xB9: { // SPARKLOR orange
        // The coloured sparkle streams: a CProjectileSparkle with its cell
        // swapped to the "TRAVEL" dot and the trail colour stamped
        // (m_sparkleColor indexes the "STTRAVL1" colour-table bitmap rows).
        // Stone/black/chromatic skip the palette-bitmap request; the launch
        // sound varies per colour and is empty for most.
        CProjectileSparkle* pSparkle = new CProjectileSparkle(CVidPalette::TYPE_RESOURCE);
        pSparkle->SetVidCell(CResRef("TRAVEL"));
        SHORT sparkleColor = 0;
        CResRef fireSoundRef("");
        BOOL requestPalette = TRUE;
        switch (projectileType) {
        case 0x2F: sparkleColor = 2;   fireSoundRef = "TRA_04A"; break;
        case 0x30: sparkleColor = 4;   fireSoundRef = "TRA_04C"; break;
        case 0x31: sparkleColor = 6;   break;
        case 0x32: sparkleColor = 9;   fireSoundRef = "TRA_01";  break;
        case 0x33: sparkleColor = 0xA; requestPalette = FALSE;   break;
        case 0x34: sparkleColor = 1;   requestPalette = FALSE;   break;
        case 0x35: sparkleColor = 3;   requestPalette = FALSE;   break;
        case 0x36: sparkleColor = 7;   break;
        case 0x37: sparkleColor = 5;   fireSoundRef = "TRA_04D"; break;
        case 0xB8: sparkleColor = 0xB; fireSoundRef = "TRA_04A"; break;
        case 0xB9: sparkleColor = 0xC; fireSoundRef = "TRA_03";  break;
        }
        if (requestPalette) {
            pSparkle->SetTravelPalette(CString("STTRAVL1"));
        }
        pSparkle->m_bSparkleTrail = TRUE;
        pSparkle->m_sparkleColor = sparkleColor;
        pSparkle->m_fireSoundRef = fireSoundRef;
        pSparkle->m_bHasHeight = TRUE;
        pSparkle->m_arrivalSoundRef = CResRef("");
        pProjectile = pSparkle;
        break;
    }

    case 0x41: {
        // GAZE -- an invisible sparkle carrier at double speed (4x the BAM
        // base velocity after the constructor's own doubling); delivers its
        // payload with no visible missile.
        CProjectileSparkle* pSparkle = new CProjectileSparkle(CVidPalette::TYPE_RESOURCE);
        pSparkle->SetVidCell(CResRef("TRAVEL"));
        pSparkle->SetTravelPalette(CString("STTRAVL1"));
        pSparkle->m_visible = 0;
        pSparkle->m_bSparkleTrail = TRUE;
        pSparkle->m_sparkleColor = 0xA;
        pSparkle->m_bHasHeight = TRUE;
        pSparkle->m_velocity = static_cast<SHORT>(pSparkle->m_velocity << 1);
        pSparkle->m_fireSoundRef = CResRef("TRA_04C");
        pSparkle->m_arrivalSoundRef = CResRef("");
        pProjectile = pSparkle;
        break;
    }

    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
        // SPMAGMIS -- Magic Missile launcher; band count = projectileType - 0x43
        // (1..5 missiles), palette flag 1.
        pProjectile = new CProjectileSPMAGMIS(
            static_cast<SHORT>(projectileType - 0x43), 1);
        break;

    case 0x6F:
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
    case 0x76: {
        // Spell-school casting-glow overlays.
        CResRef visualResRef;
        switch (projectileType) {
        case 0x6F: visualResRef = "NecroCG"; break;
        case 0x70: visualResRef = "AlterCG"; break;
        case 0x71: visualResRef = "EnchaCG"; break;
        case 0x72: visualResRef = "AbjurCG"; break;
        case 0x73: visualResRef = "IllusCG"; break;
        case 0x74: visualResRef = "ConjuCG"; break;
        case 0x75: visualResRef = "InvocCG"; break;
        case 0x76: visualResRef = "DivinCG"; break;
        }
        BYTE sequenceDelay = castDelay ? castDelay : 0x32;
        pProjectile = new CProjectileBAM(visualResRef, CResRef(""), sequenceDelay, 0, visualEffect);
        break;
    }

    case 0x121:
    case 0x122:
    case 0x123:
    case 0x124:
    case 0x125: {
        // Summon-group VFX overlays (mirror CreateSummonGroupProjectile):
        // copy-from-back tint, EFF_M13 arrival sound, offset above the target.
        CResRef visualResRef;
        switch (projectileType) {
        case 0x121: visualResRef = "MSumm1X"; break;
        case 0x122: visualResRef = "ASumm1X"; break;
        case 0x123: visualResRef = "CEElemX"; break;
        case 0x124: visualResRef = "CFElemX"; break;
        case 0x125: visualResRef = "CWElemX"; break;
        }
        visualEffect.SetCopyFromBack(TRUE);
        CProjectileSummonVFX* pSummon = new CProjectileSummonVFX(visualResRef, visualEffect);
        pSummon->SetArrivalSound(CResRef("EFF_M13"));
        pSummon->SetOffsetAboveTarget(TRUE);
        pProjectile = pSummon;
        break;
    }

    case 0x15B:
    case 0x161:
    case 0x162: {
        // Single spell-hit VFX overlays: copy-from-back tint only.
        CResRef visualResRef;
        switch (projectileType) {
        case 0x15B: visualResRef = "PortalH"; break;
        case 0x161: visualResRef = "IllusH"; break;
        case 0x162: visualResRef = "CCDamaH"; break;
        }
        visualEffect.SetCopyFromBack(TRUE);
        pProjectile = new CProjectileSummonVFX(visualResRef, visualEffect);
        break;
    }

    case 0x26:
        // Fireball (SPWI304): the large area spell-hit projectile. The binary
        // builds it bare (case 0x26 = `new CProjectileFireball()`); all
        // configuration is in the leaf ctor (0x571E80). It travels, arrives and
        // fires the AOE strike through the inherited IcewindCProjectileSpellHit
        // virtuals -- AIUpdate (slot 3) -> GatherTargets (36) -> Strike (37) ->
        // StrikeTarget (38), plus OnArrival (28)/Fire (27) -- all now recovered,
        // so each victim gets a carrier loaded with a copy of the effect list on
        // detonation.
        pProjectile = new CProjectileFireball();
        break;

    case 0xFF: {
        // Enchantment-school area spell-hit carrier (projectile 255): SPWI420
        // Emotion: Fear and the other leaf-less point-cast Enchantment AOE
        // spells.  The binary (0x525bcd) builds the base spell-hit projectile
        // directly -- no dedicated leaf -- with nType 0x96, then stamps
        // emission slot 0's Enchantment detonation cell (EnchanX) and area
        // sound (ARE_M21) and arms copy-from-back on its visual.
        //
        // Without this case the missile fell to the default plain CProjectile,
        // whose Fire() (0x78E740) is a stub, so the targetType-2 gameplay
        // effects the caster attached in ForceSpellPointAction (State: Horror,
        // the save rider, ...) never travelled, gathered, or delivered -- the
        // creatures in the AOE were never affected.  The base spell-hit family
        // arrives instantly (m_bHasTravelCell = 0), detonates, and its
        // AIUpdate -> GatherTargets -> Strike -> StrikeTarget pass spawns a
        // per-victim carrier loaded with a copy of the effect list.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x96);
        pSpellHit->m_visual1.m_cellResRef.Set("EnchanX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M21");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    // The following leaf-less point-cast AOE spell-hit cases share case 0xFF's shape:
    // a projectile type our switch lacked, so the missile fell to the default plain
    // CProjectile (stub Fire()), silently dropping the caster's targetType-2 gameplay
    // effects. Each reproduces its binary jump-table handler and delivers through the
    // recovered IcewindCProjectileSpellHit strike chain (proven by case 0xFF).
    //
    // The sibling travelling-VFX carriers (Icelance 0xFB, Otiluke 0x10D, Vitriolic
    // 0x13C -- IcewindCProjectileTravellingVFX) are deliberately NOT recovered here:
    // routing an effect-carrying spell through that class crashes on arrival (its
    // CProjectileTravelling delivery path is only partly recovered).  Left for a
    // dedicated fix rather than shipping a crash -- missing beats wrong.

    case 0xF0: {
        // See Invisibility (SPWI203) / Invisibility Purge (SPPR309): base spell-hit
        // carrier (0x525725, nType 0x12c). Seeds the strike filter from the caster's
        // allegiance, then stamps slot 0's detonation cell (Area2X) + area sound
        // (EFF_M99) + copy-from-back.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12c);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("Area2X");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_M99");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0xFD: {
        // Priest/innate area spell-hit carrier (0x525b26, nType 0x258): SPPR313,
        // SPIN209 and siblings. Stamps slot 0's detonation cell (Area1X) + impact
        // sound (EFF_P99) + copy-from-back.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x258);
        pSpellHit->m_visual1.m_cellResRef.Set("Area1X");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_P99");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x104: {
        // Priest/innate area spell-hit carrier (0x525d03, nType 0x258): SPPR414,
        // SPIN210. Twin of case 0xFD (Area1X / EFF_P99 / copy-from-back).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x258);
        pSpellHit->m_visual1.m_cellResRef.Set("Area1X");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_P99");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x13E: {
        // Horrid Wilting (SPWI805): Necromancy area spell-hit carrier (0x5275b6,
        // nType 0x12c). Twin of case 0xFF, differing only in nType and the resrefs:
        // detonation cell ADHWilX + area sound ARE_M17 + copy-from-back.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12c);
        pSpellHit->m_visual1.m_cellResRef.Set("ADHWilX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M17");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x151: {
        // Holy Smite (SPIN203 innate / SPPR324 priest, strref 3925): radiant-burst
        // area spell-hit carrier (0x527a18, nType 0xC8). Shared tail 0x527abe stamps
        // emission slot 3's detonation cell (HSmiteA) with a transparent (0x40)
        // visual, the fan-emission respawn/anim flags and 0x12C density, and the
        // recurring strike clock (period 0x2710, interval 0xA) over a 0x2D-tick life.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->m_visual3.m_cellResRef.Set("HSmiteA");
        pSpellHit->m_visual3.m_fx.SetTransparency(TRUE, 0x40);
        pSpellHit->m_visual3RespawnFlag = 1;
        pSpellHit->m_visual3AnimMode = 1;
        pSpellHit->m_visual3DensityBase = 0x12C;
        pSpellHit->m_strikePeriod = 0x2710;
        pSpellHit->m_strikeCountdown = 0;
        pSpellHit->m_strikeInterval = 0xA;
        pSpellHit->m_lifetime = 0x2D;
        pProjectile = pSpellHit;
        break;
    }

    case 0x152: {
        // Unholy Blight (SPPR325, strref 4346): the evil twin of Holy Smite --
        // identical shared-tail config (0x527a6c -> 0x527abe) but detonation cell
        // UBlighA. nType 0xC8.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->m_visual3.m_cellResRef.Set("UBlighA");
        pSpellHit->m_visual3.m_fx.SetTransparency(TRUE, 0x40);
        pSpellHit->m_visual3RespawnFlag = 1;
        pSpellHit->m_visual3AnimMode = 1;
        pSpellHit->m_visual3DensityBase = 0x12C;
        pSpellHit->m_strikePeriod = 0x2710;
        pSpellHit->m_strikeCountdown = 0;
        pSpellHit->m_strikeInterval = 0xA;
        pSpellHit->m_lifetime = 0x2D;
        pProjectile = pSpellHit;
        break;
    }

    case 0x175: {
        // Control Undead (SPWI717) / Mass Dominate (SPWI911): Enchantment domination
        // area spell-hit carrier (0x5286ef, nType 0xC8). Like case 0xFF: detonation
        // cell EnchanX + area sound ARE_M21 + copy-from-back.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->m_visual1.m_cellResRef.Set("EnchanX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M21");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    // The school-detonation spell-hit cases.  Every one of them builds the bare
    // carrier, optionally seeds the strike filter from the caster's allegiance --
    // SetStrikeTargetFilter for the offensive spells, SetStrikeAllyFilter for the
    // beneficial ones -- and stamps emission slot 0 with the school's detonation
    // cell (Area1X..Area4X for the generic bursts, AbjuraX / AlteraX / EnchanX for
    // the school-coloured ones), the area sound, and copy-from-back.  Same shape as
    // case 0xFF above; they differ only in nType (the gather radius), the filter and
    // the two resrefs.  All of them fell to the default plain CProjectile until now.

    case 0xEC: {
        // Bless (SPPR101, SPITM02).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xFA);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("Area1X");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_P99");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0xED: {
        // Bane (SPPR111): the hostile twin of Bless -- same radius and detonation,
        // the enemy-side filter (0x5255D7 -> 0x52559D, case 0xEC's tail).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xFA);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("Area1X");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_P99");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0xEE: {
        // Remove Fear (SPPR108, SPIN198): Abjuration detonation on the caster's side.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("AbjuraX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M20");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0xEF: {
        // Detect Evil (SPIN120): no filter -- the gather pass takes everyone in range.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->m_visual1.m_cellResRef.Set("Area2X");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_M99");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0xF1: {
        // Horror (SPWI205).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x96);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("Area4X");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_M99");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0xF2: {
        // Resist Fear (SPWI210): twin of case 0xEE (0x5257DC -> 0x52565A).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("AbjuraX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M20");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0xF3: {
        // Chant (SPPR203): the widest of the group (nType 0x15E), no filter.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x15E);
        pSpellHit->m_visual1.m_cellResRef.Set("Area1X");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_P99");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0xF5: {
        // Silence, 15' Radius (SPPR211, SPPR988): Alteration detonation, no filter.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x96);
        pSpellHit->m_visual1.m_cellResRef.Set("AlteraX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M19");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0xF7: {
        // Ally-side Alteration burst (0x525953 -> 0x525B07, case 0xFC's tail). No
        // .SPL in the shipped game selects this projectile.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("AlteraX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M19");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0xF8: {
        // Slow (SPWI312): the hostile twin of case 0xF7.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("AlteraX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M19");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0xFA: {
        // Remove Paralysis (SPPR308, SPIN248): Abjuration burst on the caster's side
        // (0x525A0B -> 0x52566D, case 0xEE's tail).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("AbjuraX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M20");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0xFC: {
        // Strength of One (SPPR312).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xFA);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("AlteraX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M19");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0xFE: {
        // Confusion (SPWI401): Enchantment burst, enemy-side (0x525B68 -> 0x52872C,
        // the EnchanX / ARE_M21 tail case 0xFF also lands on).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("EnchanX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M21");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x100: {
        // Malison (SPWI412): same shape as case 0xFE.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("EnchanX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M21");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x101: {
        // Defensive Harmony (SPPR406): the tightest radius of the group (nType 0x64),
        // ally-side.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x64);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("EnchanX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M21");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x102: {
        // Magic Circle Against Evil (SPPR408) / Holy Aura (SPPR801).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("AbjuraX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M20");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x108: {
        // Chaos (SPWI509): Enchantment burst, enemy-side (0x525DF0 -> 0x525C36,
        // case 0x100's tail).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("EnchanX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M21");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x10A: {
        // Righteous Wrath of the Faithful (SPPR511, SPITM05, EFFRWF1/EFFRWF2):
        // the widest ally burst (nType 0x15E). The binary stamps the detonation
        // before seeding the filter (0x525E27); order kept.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x15E);
        pSpellHit->m_visual1.m_cellResRef.Set("AbjuraX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M20");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pProjectile = pSpellHit;
        break;
    }

    case 0x111: {
        // Mass Invisibility (SPWI709).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("Area3X");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_M99");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x113: {
        // Confusion (SPPR709) / Gram's Paradox (SPIN239) / EFFGPX. Shares the arm
        // at 0x525B98 with case 0x153 -- both blocks are a bare jmp into it.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("EnchanX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M21");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x114: {
        // Symbol of Pain (SPPR714) / Symbol of Stunning (SPPR728).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("SoPainX");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_P49");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x115: {
        // Symbol of Hopelessness (SPPR715) / Symbol of Fear (SPPR727).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("SoHopeX");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_P50");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x150: {
        // Power Word, Blind (SPWI808).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x96);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("PWStunH");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_M38");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x153: {
        // Greater Command (SPPR518): twin of case 0x113, same shared arm.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("EnchanX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M21");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x154:
        // No .SPL in the shipped game selects projectile 0x154; it configures
        // exactly like case 0x155 below (0x527B3E -> 0x526279, case 0x115's tail
        // entered past the filter call).
    case 0x155: {
        // Holy Word (SPPR721) / Blasphemy (SPPR722): unfiltered -- the detonation
        // gathers everyone in range, and the effect list does the sorting.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->m_visual1.m_cellResRef.Set("SoHopeX");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_P50");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x166: {
        // Invisibility Sphere (SPWI321): the tight ally sphere (nType 0x7D).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x7D);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("Area1X");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_M99");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x167: {
        // Healing Circle (SPPR521) / Mass Heal (SPPR802).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("Area1X");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_P99");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x16A: {
        // Wail of the Banshee (SPWI906): the only case in the family whose area
        // sound is EFF_W99.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("Area1X");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_W99");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x16B:
    case 0x16C: {
        // Two jump-table entries onto the same block (0x52831D). No .SPL in the
        // shipped game selects either; the configuration is the filtered twin of
        // case 0x155.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->SetStrikeTargetFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("SoHopeX");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_P50");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x16F: {
        // Mass Haste (SPWI623).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("AlteraX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M19");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x173: {
        // Aura of Vitality (SPPR729): the tightest radius in the family (nType 0x46).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x46);
        pSpellHit->SetStrikeAllyFilter(pCaster);
        pSpellHit->m_visual1.m_cellResRef.Set("AlteraX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M19");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x174: {
        // Banishment (SPWI718): unfiltered Abjuration burst (0x52869F -> 0x525674,
        // case 0xEE's tail entered past the filter call).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xA0);
        pSpellHit->m_visual1.m_cellResRef.Set("AbjuraX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M20");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pProjectile = pSpellHit;
        break;
    }

    case 0x17D: {
        // Breathes Acid (SPIN225): a dragon breath rather than a cast spell, and
        // the only case in this group that also drives emission slot 2 -- the
        // GABreaA fan re-emitting over a 0x2D-tick life on the recurring strike
        // clock, on top of the GABreaX detonation in slot 0.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->m_visual1.m_cellResRef.Set("GABreaX");
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_M21");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->m_visual3.m_cellResRef.Set("GABreaA");
        pSpellHit->m_visual3.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->m_visual3AnimMode = 1;
        pSpellHit->m_visual3AnimFlag36 = 1;
        pSpellHit->m_visual3DensityBase = 0x1F4;
        pSpellHit->m_strikePeriod = 0x2710;
        pSpellHit->m_strikeCountdown = 0;
        pSpellHit->m_strikeInterval = 0xA;
        pSpellHit->m_lifetime = 0x2D;
        pSpellHit->m_aoeRange = 0xC8;   // re-stores the value the ctor already set
        pProjectile = pSpellHit;
        break;
    }

    // The persistent-cloud spell-hit cases.  Same bare carrier, but the
    // configuration lands on emission slot 2 (m_visual3, the re-emitting fan) or
    // slot 1 (m_visual2) instead of slot 0, and most of them arm the recurring
    // strike clock so the cloud keeps striking whatever stands in it for the whole
    // of m_lifetime.  All fell to the default plain CProjectile until now.

    case 0x11A: {
        // Releases Acidic Vapor (SPIN993, strref 4388): a creature attack rather
        // than a cast spell -- it announces itself before building the carrier.
        IcewindMisc::DisplayFeedbackMessage(static_cast<CGameSprite*>(pCaster), 4388, 0);
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x64);
        pSpellHit->m_visual2.m_cellResRef.Set("SCloudR");
        pSpellHit->m_visual2.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->m_visual2AnimMode = 1;
        pSpellHit->m_visual2MaxSpawn = 7;
        pProjectile = pSpellHit;
        break;
    }

    case 0x11E: {
        // Incendiary cloud, second stage (SPIN983).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x96);
        pSpellHit->m_visual3.m_cellResRef.Set("ICloudB");
        pSpellHit->m_visual3.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->m_visual3AnimMode = 1;
        pSpellHit->m_visual3AnimFlag36 = 1;
        pSpellHit->m_visual3DensityBase = 0x1F4;
        pProjectile = pSpellHit;
        break;
    }

    case 0x11F: {
        // Incendiary cloud, first stage (SPIN981, SPIN982): respawns from the pool
        // instead of animating a flag, and outlives 0x11E (0xC8 ticks).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x96);
        pSpellHit->m_visual3.m_cellResRef.Set("ICloudA");
        pSpellHit->m_visual3.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->m_visual3AnimMode = 1;
        pSpellHit->m_visual3RespawnFlag = 1;
        pSpellHit->m_visual3DensityBase = 0x1F4;
        pSpellHit->m_lifetime = 0xC8;
        pProjectile = pSpellHit;
        break;
    }

    case 0x12B: {
        // Soul Eater (SPWI619) / Orrick's Soul Devourer (SPIN969).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x78);
        pSpellHit->m_visual3.m_cellResRef.Set("SEaterA");
        pSpellHit->m_visual3.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->m_visual3AnimMode = 1;
        pSpellHit->m_visual3RespawnFlag = 1;
        pSpellHit->m_visual3.m_soundResRef.Set("ARE_M18");
        pSpellHit->m_visual3DensityBase = 0xC8;
        pProjectile = pSpellHit;
        break;
    }

    case 0x12C: {
        // Spike Growth (SPPR320, SPIN206): the longest-lived cloud in the family
        // (0x7D0 ticks) re-striking every tenth pass.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x96);
        pSpellHit->m_visual1.m_cellResRef.Set("SGrowtX");
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_P111");
        pSpellHit->m_visual1.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->m_visual3.m_cellResRef.Set("SGrowtA");
        pSpellHit->m_visual3.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->m_visual3AnimMode = 1;
        pSpellHit->m_visual3RespawnFlag = 1;
        pSpellHit->m_visual3.m_soundResRef.Set("AFT_P24");
        pSpellHit->m_visual3DensityBase = 0xFA;
        pSpellHit->m_strikePeriod = 0xA;
        pSpellHit->m_strikeCountdown = 0;
        pSpellHit->m_strikeInterval = 0xA;
        pSpellHit->m_lifetime = 0x7D0;
        pProjectile = pSpellHit;
        break;
    }

    case 0x12D: {
        // Rainstorm (SPPR221).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->m_visual3.m_cellResRef.Set("CloudBA");
        pSpellHit->m_visual3.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->m_visual3AnimMode = 1;
        pSpellHit->m_visual3RespawnFlag = 1;
        pSpellHit->m_visual3.m_soundResRef.Set("ARE_P24");
        pSpellHit->m_visual3DensityBase = 0x64;
        pSpellHit->m_strikePeriod = 0xA;
        pSpellHit->m_strikeCountdown = 0;
        pSpellHit->m_strikeInterval = 0xA;
        pSpellHit->m_lifetime = 0xC8;
        pProjectile = pSpellHit;
        break;
    }

    case 0x133: {
        // Mist of Eldath (SPPR720): the only case here that stamps slot 0's sound
        // without a slot-0 cell -- the detonation is silent-visual, audible-only.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x78);
        pSpellHit->m_visual1.m_soundResRef.Set("EFF_P104");
        pSpellHit->m_visual3.m_cellResRef.Set("MoEldaA");
        pSpellHit->m_visual3.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->m_visual3AnimMode = 1;
        pSpellHit->m_visual3RespawnFlag = 1;
        pSpellHit->m_visual3DensityBase = 0x4B0;
        pProjectile = pSpellHit;
        break;
    }

    case 0x135: {
        // Cloud of Pestilence (SPPR417): a half-transparent (0x80) fan rather than
        // a copy-from-back one. Shares case 0x13D's clock tail (0x52707B -> 0x52758C).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->m_visual3.m_cellResRef.Set("CoPestA");
        pSpellHit->m_visual3.m_fx.SetTransparency(TRUE, 0x80);
        pSpellHit->m_visual3AnimMode = 1;
        pSpellHit->m_visual3RespawnFlag = 1;
        pSpellHit->m_visual3.m_soundResRef.Set("ARE_P25");
        pSpellHit->m_visual3DensityBase = 0xAF;
        pSpellHit->m_strikePeriod = 0xA;
        pSpellHit->m_strikeCountdown = 0;
        pSpellHit->m_strikeInterval = 0xA;
        pSpellHit->m_lifetime = 0x190;
        pProjectile = pSpellHit;
        break;
    }

    case 0x136: {
        // Undead Ward (SPPR516): sound only -- no cell in any slot -- and the widest
        // re-strike interval in the family (0x2710), so the ward strikes once and
        // then holds for its 0x3E8-tick life.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->m_visual1.m_soundResRef.Set("ARE_P28");
        pSpellHit->m_visual3.m_soundResRef.Set("AFT_P27");
        pSpellHit->m_strikePeriod = 0xA;
        pSpellHit->m_strikeCountdown = 0;
        pSpellHit->m_strikeInterval = 0x2710;
        pSpellHit->m_lifetime = 0x3E8;
        pProjectile = pSpellHit;
        break;
    }

    case 0x13D: {
        // Suffocate (SPWI711).
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x78);
        pSpellHit->m_visual3.m_cellResRef.Set("SuffocA");
        pSpellHit->m_visual3.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->m_visual3AnimMode = 1;
        pSpellHit->m_visual3RespawnFlag = 1;
        pSpellHit->m_visual3.m_soundResRef.Set("AFT_M18");
        pSpellHit->m_visual3DensityBase = 0x3E8;
        pSpellHit->m_strikePeriod = 0xA;
        pSpellHit->m_strikeCountdown = 0;
        pSpellHit->m_strikeInterval = 0xA;
        pSpellHit->m_lifetime = 0x190;
        pProjectile = pSpellHit;
        break;
    }

    case 0x15F: {
        // Releases Jelly Vapor (SPIN182, strref 27030): a creature attack, and the
        // twin of case 0x11A -- it lands on slot 1 through the same tail (0x527F29).
        IcewindMisc::DisplayFeedbackMessage(static_cast<CGameSprite*>(pCaster), 27030, 0);
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x7D);
        pSpellHit->m_visual2.m_cellResRef.Set("CloudKR");
        pSpellHit->m_visual2.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->m_visual2AnimMode = 1;
        pSpellHit->m_visual2MaxSpawn = 7;
        pProjectile = pSpellHit;
        break;
    }

    case 0x16E: {
        // Meteor Swarm (SPWI908): the fan neither respawns from the pool nor
        // animates, and the carrier is pinned to a single direction.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
        pSpellHit->m_visual3.m_cellResRef.Set("MSwarmA");
        pSpellHit->m_visual3.m_soundResRef.Set("ARE_P03");
        pSpellHit->m_visual3.m_fx.SetCopyFromBack(TRUE);
        pSpellHit->m_visual3RespawnFlag = 0;
        pSpellHit->m_visual3AnimMode = 0;
        pSpellHit->m_visual3DensityBase = 0xFA;
        pSpellHit->m_strikePeriod = 0xA;
        pSpellHit->m_strikeCountdown = 0;
        pSpellHit->m_strikeInterval = 0x2710;
        pSpellHit->m_lifetime = 0x2D;
        pSpellHit->m_dirCount = 1;
        pSpellHit->m_aoeRange = 0x12C;  // re-stores the value the ctor already set
        pProjectile = pSpellHit;
        break;
    }

    case 0x179: {
        // Breathes Mind Fog (SPIN221): case 0x135's cloud on a shorter life.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xC8);
        pSpellHit->m_visual3.m_cellResRef.Set("CoPestA");
        pSpellHit->m_visual3.m_fx.SetTransparency(TRUE, 0x80);
        pSpellHit->m_visual3AnimMode = 1;
        pSpellHit->m_visual3RespawnFlag = 1;
        pSpellHit->m_visual3.m_soundResRef.Set("ARE_P25");
        pSpellHit->m_visual3DensityBase = 0xAF;
        pSpellHit->m_strikePeriod = 0xA;
        pSpellHit->m_strikeCountdown = 0;
        pSpellHit->m_strikeInterval = 0xA;
        pSpellHit->m_lifetime = 0x12C;
        pProjectile = pSpellHit;
        break;
    }

    // The bare spell-hit cases: the carrier alone, at most a strike filter. The
    // spell's own effect list carries everything visible; nothing is stamped on
    // the emission slots, so these detonate invisibly and strike. Last of the
    // types that fell to the default plain CProjectile.

    case 0xF4: {
        // Find Traps (SPPR205): the only case in the family that opts into
        // non-creature targets, which is how the sweep sees traps at all.
        IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xFA);
        pSpellHit->m_bAffectNonCreatures = 1;
        pProjectile = pSpellHit;
        break;
    }

    case 0x118: {
        // No .SPL in the shipped game selects projectile 0x118. Its radius is the
        // only one the factory reads from a global rather than an immediate.
        IcewindCProjectileSpellHit* pSpellHit =
            new IcewindCProjectileSpellHit(g_nSpellHitDefaultRange);
        pProjectile = pSpellHit;
        break;
    }

    case 0x119:
        // No .SPL selects it.
        pProjectile = new IcewindCProjectileSpellHit(0xFA);
        break;

    case 0x120:
        // SPIN975.
        {
            IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x12C);
            pSpellHit->SetStrikeTargetFilter(pCaster);
            pProjectile = pSpellHit;
        }
        break;

    case 0x127:
        // Horror (SPWI025) -- the innate/item form of case 0xF1's spell, with no
        // detonation visual of its own (0x5268F3 -> the filter-only tail 0x528872).
        {
            IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0xFA);
            pSpellHit->SetStrikeTargetFilter(pCaster);
            pProjectile = pSpellHit;
        }
        break;

    case 0x140:
        // Mournful Wail (SPIN134).
        pProjectile = new IcewindCProjectileSpellHit(0xE1);
        break;

    case 0x141:
        // Death Knell (SPIN135).
        pProjectile = new IcewindCProjectileSpellHit(0xFA);
        break;

    case 0x142:
        // No .SPL selects it.
        pProjectile = new IcewindCProjectileSpellHit(0x96);
        break;

    case 0x143:
        // Undying Lament (SPIN137).
        pProjectile = new IcewindCProjectileSpellHit(0xC8);
        break;

    case 0x156:
        // Great Roar (SPIN138): the widest gather in the whole family (nType 0x1F4),
        // filtered to the roarer's enemies (0x527BCC -> 0x528872).
        {
            IcewindCProjectileSpellHit* pSpellHit = new IcewindCProjectileSpellHit(0x1F4);
            pSpellHit->SetStrikeTargetFilter(pCaster);
            pProjectile = pSpellHit;
        }
        break;

    case 0x180:
    case 0x181:
        // Lake of Fire (SPIN254) and Acid Bath (SPIN253) share one block
        // (0x528CF7): the tightest gather in the family (nType 0x41).
        pProjectile = new IcewindCProjectileSpellHit(0x41);
        break;

    case 0x5F:
        // Stinking Cloud (SPWI213): the persistent-gas area spell-hit projectile.
        // Sibling of Fireball -- another bare IcewindCProjectileSpellHit leaf; all
        // configuration is in the leaf ctor (0x574B80). The inherited SpellHit AOE
        // strike (AIUpdate -> GatherTargets -> Strike -> StrikeTarget) is recovered
        // and the detonation visual fires through the inherited OnArrival; the
        // persistent gas-area overlay (ARE_M02) rides the same periodic re-strike
        // clock (m_strikePeriod/m_strikeInterval). Previously this type fell through
        // to the default `new CProjectile()`, so casting Stinking Cloud produced
        // nothing.
        pProjectile = new CProjectileStinkingCloud();
        break;

    case 0x40:
        // Web (SPWI215): the persistent web-field area spell-hit projectile. Another
        // bare IcewindCProjectileSpellHit leaf (ctor 0x574EB0); the entangling web area
        // (ARE_M03) and the inherited SpellHit strike fire through the family virtuals.
        // Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileWeb();
        break;

    case 0x62:
        // Ice Storm (SPWI404): the pounding-hail area spell-hit projectile. Bare
        // IcewindCProjectileSpellHit leaf (ctor 0x573460); the ice area (ARE_M04) and
        // the inherited strike fire through the family virtuals. Previously fell through
        // to the default plain CProjectile.
        pProjectile = new CProjectileIceStorm();
        break;

    case 0xEB:
        // Entangle (SPPR105): the grasping-undergrowth area spell-hit projectile. Bare
        // IcewindCProjectileSpellHit leaf (ctor 0x571CE0); the entangling area (ARE_P01)
        // and the inherited strike fire through the family virtuals. Previously fell
        // through to the default plain CProjectile.
        pProjectile = new CProjectileEntangle();
        break;

    case 0x5C:
        // Fire Storm (SPPR705; the projectile is shared by SPWI081/SPWI399): the
        // persistent fire area spell-hit projectile. Bare IcewindCProjectileSpellHit
        // leaf (ctor 0x572290). Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileFireStorm();
        break;

    case 0xD3:
        // Acid Storm (SPWI708): the persistent acid area spell-hit projectile. Bare
        // IcewindCProjectileSpellHit leaf (ctor 0x571170). Previously fell through to
        // the default plain CProjectile.
        pProjectile = new CProjectileAcidStorm();
        break;

    case 0xD5:
        // Spike Stones (SPPR512): the persistent spike-field area spell-hit projectile.
        // Bare IcewindCProjectileSpellHit leaf (ctor 0x5747A0). Previously fell through
        // to the default plain CProjectile.
        pProjectile = new CProjectileSpikeStones();
        break;

    case 0x116:
        // Power Word, Kill (SPWI903): the single-burst spell-hit projectile. Bare
        // IcewindCProjectileSpellHit leaf (ctor 0x573E90). Previously fell through to
        // the default plain CProjectile.
        pProjectile = new CProjectilePowerWordKill();
        break;

    case 0x16D:
        // Symbol of Death (SPPR726): the single-burst spell-hit projectile. Bare
        // IcewindCProjectileSpellHit leaf (ctor 0x5778A0). Previously fell through to
        // the default plain CProjectile.
        pProjectile = new CProjectileSymbolOfDeath();
        break;

    case 0xBB:
        // Cloudkill (SPWI018): the persistent poison-gas area spell-hit projectile, a
        // full three-slot cloud leaf (ctor 0x571310). Previously fell through to the
        // default plain CProjectile.
        pProjectile = new CProjectileCloudkill();
        break;

    case 0xD4:
        // Acid Fog (SPWI019): the persistent acid-gas area spell-hit projectile, a
        // sibling of Cloudkill (ctor 0x5714E0). Previously fell through to the default
        // plain CProjectile.
        pProjectile = new CProjectileAcidFog();
        break;

    case 0x65:
        // Grease (SPWI101): the persistent grease-slick area spell-hit projectile (ctor
        // 0x572670). Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileGrease();
        break;

    case 0x10B:
        // Circle of Death (SPWI606): the single-burst spell-hit projectile (ctor
        // 0x571930). Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileCircleOfDeath();
        break;

    case 0xD8:
        // Insect Plague (SPPR510): the persistent insect-swarm area spell-hit projectile,
        // a full three-slot cloud leaf (ctor 0x573AF0). Previously fell through to the
        // default plain CProjectile.
        pProjectile = new CProjectileInsectPlague();
        break;

    case 0xD9:
        // Snilloc's Snowball Swarm (SPWI220): the visible travelling area spell-hit
        // projectile in the Fireball mould (ctor 0x5745E0). Previously fell through to
        // the default plain CProjectile.
        pProjectile = new CProjectileSnowballSwarm();
        break;

    case 0xD6:
        // Fiery Cloud (SPWI802): the persistent fire-cloud area spell-hit projectile and
        // the one effect that drives the cloud-flip particle path (ctor 0x573600).
        // Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileFieryCloud();
        break;

    case 0xD7:
        // Produce Fire (SPPR411): the persistent fire area spell-hit projectile (ctor
        // 0x574140). Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileProduceFire();
        break;

    case 0x132:
        // Tremor (SPPR719): the minimal area spell-hit projectile (ctor 0x575B90).
        // Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileTremor();
        break;

    case 0xF6:
        // Dispel Magic (EFFDM1): the single-burst abjuration-glow spell-hit projectile
        // (ctor 0x577B20). Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileDispelMagic();
        break;

    case 0x2A:
        // Sleep (SPWI116): enchantment-glow spell-hit overlay (ctor 0x574300).
        // Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileSleep();
        break;

    case 0xF9:
        // Hold Animal (SPPR305): enchantment-glow spell-hit overlay (ctor 0x572830).
        // Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileHoldAnimal();
        break;

    case 0xBE:
        // Eye of Stone (SPIN132): enchantment-glow spell-hit overlay (ctor 0x572E60).
        // Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileEyeOfStone();
        break;

    case 0x165:
        // Halt Undead (SPWI320): enchantment-glow spell-hit overlay (ctor 0x573160).
        // Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileHaltUndead();
        break;

    case 0x43:
        // Flame Strike (projectile type 67, SPIN977): the sound-only spell-hit overlay
        // ("EFF_P16", no cell) (ctor 0x572470). Previously fell through to the default
        // plain CProjectile.
        pProjectile = new CProjectileFlameStrike();
        break;

    case 0x107:
        // Hold Monster (projectile type 263): the enchantment-glow spell-hit overlay
        // ("EnchanX"/"ARE_M21"), a sibling of Hold Animal (ctor 0x572B30). Previously fell
        // through to the default plain CProjectile.
        pProjectile = new CProjectileHoldMonster();
        break;

    case 0x10E:
        // Fire Seed (projectile type 270): the visible travelling "MagicStn" carrier that
        // bursts the "FSeedsX"/"EFF_P45" overlay (ctor 0x572110). Previously fell through to
        // the default plain CProjectile.
        pProjectile = new CProjectileFireSeed();
        break;

    case 0x117:
        // Malavon's Corrosive Fog (projectile type 279): a full three-slot cloud reusing the
        // Death Fog visuals ("DFogX"/"DFogR"/"DFogA"), a sibling of Acid Fog with a longer
        // 2000-tick lifetime and 0xFA range (ctor 0x573CC0). Previously fell through to the
        // default plain CProjectile.
        pProjectile = new CProjectileCorrosiveFog();
        break;

    case 0x126:   // Portal Animation Flipping Hack Open  (projectile type 294)
    case 0x129:   // Portal Animation Flipping Hack Close (projectile type 297)
        // The minimal portal-door animation overlay: base lifetime 2000, target type reset to
        // NOT_SPRITE, nothing else (ctor 0x5756C0, shared by both directions). Not a spell, which
        // is why no SPL owns it. Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectilePortalAnimFlip();
        break;

    case 0x17F:
        // Boulder, Big (Trap) (projectile type 383, SPWI088): the visible travelling
        // "BIGBOLDR" carrier that loops "AM6103e"/"AM5101e" without a burst overlay (ctor
        // 0x577590). Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileBigBoulder();
        break;

    case 0x168:
        // Delayed Blast Fireball (SPWI714): the Fireball-shaped carrier whose OnArrival
        // latches instead of detonating and whose AIUpdate proximity-delays the blast
        // (ctor 0x576990). Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileDBFireball();
        break;

    case 0x178:
        // Turn Undead (projectile type 376): the minimal no-visual spell-hit overlay (ctor
        // 0x576CF0). Previously fell through to the default plain CProjectile.
        pProjectile = new CProjectileTurnUndead();
        break;

    case 0x28: {
        // Lightning Bolt ("LightnT" BAM): SPWI002/308/997 and Eye of the Mage.
        // The line beam that rakes everything it crosses; height on, range 400.
        CProjectileLightningBolt* pBolt = new CProjectileLightningBolt(CResRef("LightnT"));
        pBolt->m_beamRange = 400;
        pBolt->m_targetMap.m_nRange = 50;
        pBolt->m_velocity = 20;
        pBolt->m_visualEffect.SetCopyFromBack(TRUE);
        pBolt->m_dirCount = 16;
        pBolt->m_bHasHeight = TRUE;
        pBolt->m_useHeightOffset = 1;
        pBolt->m_fireSoundRef = CResRef("TRA_09");
        pProjectile = pBolt;
        break;
    }

    case 0x12E: {
        // Smashing Wave ("SWaveX" BAM): SPPR522. Slower (10), ground-hugging
        // (no height) and its north-half facings are not folded.
        CProjectileLightningBolt* pBolt = new CProjectileLightningBolt(CResRef("SWaveX"));
        pBolt->m_velocity = 10;
        pBolt->m_dirCount = 16;
        pBolt->m_bMirrorNorth = 0;
        pBolt->m_visualEffect.SetCopyFromBack(TRUE);
        pBolt->m_bHasHeight = FALSE;
        pBolt->m_useHeightOffset = 0;
        pBolt->m_targetMap.m_nRange = 50;
        pBolt->m_beamRange = 400;
        pBolt->m_fireSoundRef = CResRef("TRA_56");
        pProjectile = pBolt;
        break;
    }

    case 0x139: {
        // Lance of Disruption ("LoDisrT" BAM): SPWI319. Longer rake (600).
        CProjectileLightningBolt* pBolt = new CProjectileLightningBolt(CResRef("LoDisrT"));
        pBolt->m_velocity = 20;
        pBolt->m_dirCount = 16;
        pBolt->m_visualEffect.SetCopyFromBack(TRUE);
        pBolt->m_bHasHeight = TRUE;
        pBolt->m_useHeightOffset = 1;
        pBolt->m_targetMap.m_nRange = 50;
        pBolt->m_beamRange = 600;
        pBolt->m_fireSoundRef = CResRef("TRA_59");
        pProjectile = pBolt;
        break;
    }

    case 0x17A: {
        // The acid-breath weapon ("HDABreT" BAM): SPIN222 ("Breathes Acid").
        // Same shape as the lance, range 600.
        CProjectileLightningBolt* pBolt = new CProjectileLightningBolt(CResRef("HDABreT"));
        pBolt->m_velocity = 20;
        pBolt->m_dirCount = 16;
        pBolt->m_visualEffect.SetCopyFromBack(TRUE);
        pBolt->m_bHasHeight = TRUE;
        pBolt->m_useHeightOffset = 1;
        pBolt->m_targetMap.m_nRange = 50;
        pBolt->m_beamRange = 600;
        pBolt->m_fireSoundRef = CResRef("TRA_59");
        pProjectile = pBolt;
        break;
    }

    // ---- The cone/spray family (CProjectileCone): an empty carrier-cell name
    // and the cone's own BAM, with per-spell geometry/velocity/lifetime. ----

    case 0x16: {
        // Burning Hands ("BHandsT").
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("BHandsT"));
        pCone->m_coneLength = 200;
        pCone->m_outerRadius = 80;
        pCone->field_317 = 0;
        pCone->field_316 = 1;
        pCone->field_2EE = 16;
        pCone->m_segmentStep = 50;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pCone->m_fireSoundRef = CResRef("TRA_15");
        pProjectile = pCone;
        break;
    }

    case 0x19: {
        // Cone of Cold ("CoColdT"): wide (outer 350), thrice base velocity, a
        // short fast pulse.
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("CoColdT"));
        pCone->m_coneLength = 200;
        pCone->m_outerRadius = 350;
        pCone->field_316 = 1;
        pCone->m_velocity = static_cast<SHORT>(pCone->m_velocity * 3);
        pCone->m_segmentStep = 50;
        pCone->m_duration = 8;
        pCone->m_pulsePeriod = 4;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pCone->m_fireSoundRef = CResRef("TRA_08");
        pProjectile = pCone;
        break;
    }

    case 0x60: {
        // Skull Trap ("sklT" missile, "ShSkull" explosion).
        CProjectileSkullTrap* pSkull = new CProjectileSkullTrap(CResRef("sklT"), CResRef("ShSkull"), 0x400);
        pSkull->m_bHasHeight = TRUE;
        pSkull->m_strikeRange = 0x64;
        pSkull->m_preCheckRange = 0x32;
        pSkull->m_arrivalSoundRef = CResRef("ARE_M06");
        pSkull->m_loopArrivalSound = TRUE;
        pSkull->m_explodeSound = "EFF_M35";
        pProjectile = pSkull;
        break;
    }

    case 0x61: {
        // Color Spray ("CSprayT").
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("CSprayT"));
        pCone->m_coneLength = 200;
        pCone->m_outerRadius = 200;
        pCone->field_316 = 1;
        pCone->m_segmentStep = 30;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pCone->m_fireSoundRef = CResRef("TRA_16");
        pProjectile = pCone;
        break;
    }

    case 0x64: {
        // Glyph of Warding ("glphwrdT" missile, "ShGlyph" explosion).
        CProjectileSkullTrap* pGlyph = new CProjectileSkullTrap(CResRef("glphwrdT"), CResRef("ShGlyph"), 0x410);
        pGlyph->m_bHasHeight = TRUE;
        pGlyph->m_strikeRange = 0x50;
        pGlyph->m_preCheckRange = 0x32;
        pGlyph->m_fireSoundRef = CResRef("EFF_P22a");
        pGlyph->m_loopFireSound = TRUE;
        pGlyph->m_arrivalSoundRef = CResRef("");
        pGlyph->m_explodeSound = "EFF_P22b";
        pGlyph->m_mirror = 1;
        pProjectile = pGlyph;
        break;
    }

    case 0x110: {
        // Prismatic Spray ("PSprayT").
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("PSprayT"));
        pCone->m_coneLength = 175;
        pCone->m_outerRadius = 400;
        pCone->field_316 = 1;
        pCone->m_segmentStep = 30;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pCone->m_fireSoundRef = CResRef("TRA_21");
        pProjectile = pCone;
        break;
    }

    case 0x128: {
        // Cone of Cold variant ("CoColdT"): narrower (outer 120), twice velocity.
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("CoColdT"));
        pCone->m_coneLength = 200;
        pCone->m_outerRadius = 120;
        pCone->field_316 = 1;
        pCone->m_velocity = static_cast<SHORT>(pCone->m_velocity * 2);
        pCone->m_segmentStep = 50;
        pCone->m_duration = 8;
        pCone->m_pulsePeriod = 4;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pCone->m_fireSoundRef = CResRef("TRA_08");
        pProjectile = pCone;
        break;
    }

    case 0x12F: {
        // Long spray ("TSprayT"): length 600, thrice velocity.
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("TSprayT"));
        pCone->m_coneLength = 600;
        pCone->m_outerRadius = 300;
        pCone->field_316 = 1;
        pCone->field_317 = 0;
        pCone->field_2EE = 16;
        pCone->m_velocity = static_cast<SHORT>(pCone->m_velocity * 3);
        pCone->m_segmentStep = 35;
        pCone->m_duration = 8;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pProjectile = pCone;
        break;
    }

    case 0x13B: {
        // Shout ("ShoutT"): a wide slow sonic cone with a fast 2-tick pulse.
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("ShoutT"));
        pCone->m_coneLength = 100;
        pCone->m_outerRadius = 300;
        pCone->field_2EE = 16;
        pCone->m_velocity = static_cast<SHORT>(pCone->m_velocity * 3 / 2);
        pCone->m_segmentStep = 1000;
        pCone->field_306 = 75;
        pCone->field_30A = 0;
        pCone->m_duration = 12;
        pCone->m_pulsePeriod = 2;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pCone->m_fireSoundRef = CResRef("TRA_08");
        pProjectile = pCone;
        break;
    }

    case 0x13F: {
        // Great Shout ("GShoutT").
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("GShoutT"));
        pCone->m_coneLength = 100;
        pCone->m_outerRadius = 300;
        pCone->field_2EE = 16;
        pCone->m_velocity = static_cast<SHORT>(pCone->m_velocity * 3 / 2);
        pCone->m_segmentStep = 1000;
        pCone->field_306 = 75;
        pCone->field_30A = 0;
        pCone->m_duration = 12;
        pCone->m_pulsePeriod = 2;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pCone->m_fireSoundRef = CResRef("TRA_08");
        pProjectile = pCone;
        break;
    }

    case 0x157: {
        // Will-o-Wisp spray ("WOWispT").
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("WOWispT"));
        pCone->m_coneLength = 200;
        pCone->m_outerRadius = 200;
        pCone->field_316 = 1;
        pCone->m_segmentStep = 30;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pProjectile = pCone;
        break;
    }

    case 0x170: {
        // Frost Fingers ("FFingeT") -- same shape as Burning Hands.
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("FFingeT"));
        pCone->m_coneLength = 200;
        pCone->m_outerRadius = 80;
        pCone->field_317 = 0;
        pCone->field_316 = 1;
        pCone->field_2EE = 16;
        pCone->m_segmentStep = 50;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pCone->m_fireSoundRef = CResRef("TRA_15");
        pProjectile = pCone;
        break;
    }

    case 0x176: {
        // Shout ("ShoutT"), longer cone variant.
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("ShoutT"));
        pCone->m_coneLength = 120;
        pCone->m_outerRadius = 300;
        pCone->field_2EE = 16;
        pCone->m_velocity = static_cast<SHORT>(pCone->m_velocity * 3 / 2);
        pCone->m_segmentStep = 1000;
        pCone->field_306 = 75;
        pCone->field_30A = 0;
        pCone->m_duration = 12;
        pCone->m_pulsePeriod = 2;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pCone->m_fireSoundRef = CResRef("TRA_08");
        pProjectile = pCone;
        break;
    }

    case 0x17B: {
        // Dragon/breath fire cone ("HDFBreT"): length 300, thrice velocity.
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("HDFBreT"));
        pCone->m_coneLength = 300;
        pCone->m_outerRadius = 300;
        pCone->field_316 = 1;
        pCone->m_velocity = static_cast<SHORT>(pCone->m_velocity * 3);
        pCone->m_segmentStep = 50;
        pCone->m_duration = 8;
        pCone->m_pulsePeriod = 4;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pCone->m_fireSoundRef = CResRef("TRA_08");
        pProjectile = pCone;
        break;
    }

    case 0x17E: {
        // Great Shout ("GShoutT"), longer cone variant.
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("GShoutT"));
        pCone->m_coneLength = 120;
        pCone->m_outerRadius = 300;
        pCone->field_2EE = 16;
        pCone->m_velocity = static_cast<SHORT>(pCone->m_velocity * 3 / 2);
        pCone->m_segmentStep = 1000;
        pCone->field_306 = 75;
        pCone->field_30A = 0;
        pCone->m_duration = 12;
        pCone->m_pulsePeriod = 2;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pCone->m_fireSoundRef = CResRef("TRA_08");
        pProjectile = pCone;
        break;
    }

    case 0x182: {
        // Dragon/breath fire cone ("HDFBreT"), shorter variant (length 150).
        CProjectileCone* pCone = new CProjectileCone(CResRef(""), CResRef("HDFBreT"));
        pCone->m_coneLength = 150;
        pCone->m_outerRadius = 150;
        pCone->field_316 = 1;
        pCone->m_velocity = static_cast<SHORT>(pCone->m_velocity * 3);
        pCone->m_segmentStep = 50;
        pCone->m_duration = 8;
        pCone->m_pulsePeriod = 4;
        pCone->m_visualEffect.SetCopyFromBack(TRUE);
        pCone->m_fireSoundRef = CResRef("TRA_08");
        pProjectile = pCone;
        break;
    }

    case 0x131:
        // The wandering tornado (Whirlwind / Wing Buffet).
        pProjectile = new CProjectileWhirlwind();
        break;

    case 0x169:
        // Gate VFX overlay: no copy-from-back tint, no arrival sound.
        pProjectile = new CProjectileSummonVFX(CResRef("GateX"), visualEffect);
        break;

    case 0x2E:
        // 0x52026A: the unused sparkle slot -- the original asserts FALSE
        // (CProjectile.cpp:631) and then builds the same plain-CProjectile
        // fallback as the default case; same omitted-assert rationale.
    default:
        // 0x528DEF: unknown / unrecovered type.  The binary asserts FALSE
        // (CProjectile.cpp:3127) and then builds a plain CProjectile, so the
        // factory never returns NULL for an in-range type.  ~80 hardcoded
        // projectile classes still fall through here.
        // HACK: the assert is omitted -- the binary's UtilAssert (0x780C00)
        // shows a continueable "Run Debugger?" dialog, ours suspends and
        // shuts the game down, so asserting would kill every cast with an
        // unrecovered projectile class -- replaces 0x528DEF.
        pProjectile = new CProjectile();
        break;
    }

    // Common tail (0x528E1C): the factory stamps the 0-based projectile type on
    // every object it builds before returning it.
    if (pProjectile != NULL) {
        pProjectile->m_projectileType = projectileType - 1;
    }
    return pProjectile;
}

// 0x530790
//
// Default constructor, used by DecodeProjectile's default case (every
// subclass constructor inlines this same body before its own overrides, so
// subclasses run it implicitly here too).  The binary's store of the MFC
// nil-string sentinel (0x8C1758) into field_17E is the inlined CString
// default constructor -- our implicit member init.  The word zeroed at
// +0x9A is m_sparkleColor (the field right after the plain effect list),
// and the +0xE0/+0xE6 zeroes are m_driftDecay/m_renderFlags, declared on
// CProjectileTravelling but cleared by this base constructor.
CProjectile::CProjectile()
{
    m_nSpellLevel = 0;
    m_projectileType = 0;
    field_70 = 0;
    m_sourceId = 0;
    m_targetId = 0;
    m_sparkleColor = 0;
    m_pArea = NULL;
    m_nDeltaZ = 0;
    m_nDeltaZLast = 0;
    m_nOrigDistance = 0;
    field_17C = 0;
    m_callBackProjectile = -1;
    m_bHasHeight = FALSE;
    m_fireSoundRef = "";
    m_loopFireSound = FALSE;
    m_arrivalSoundRef = "";
    m_loopArrivalSound = FALSE;
    m_nTargetId = CGameObjectArray::INVALID_INDEX;
}

// 0x57CAC0
CProjectileBAM::CProjectileBAM(const CResRef& visualResRef, const CResRef& arrivalSoundRef, BYTE sequenceDelay, BYTE initialDelay, const IcewindCVisualEffect& visualEffect)
    : m_visualEffect(visualEffect)
{
    field_326 = FALSE;
    field_32A = 0;
    field_32B = 0;
    m_bSparkleTrail = FALSE;

    m_vidCell.SetResRef(visualResRef, FALSE, TRUE, TRUE);
    m_vidCell.FrameSet(0);
    m_sound.Stop();
    m_sound.SetResRef(arrivalSoundRef, TRUE, TRUE);
    if (!m_sound.GetLooping()) {
        m_sound.SetFireForget(TRUE);
    }
    m_sound.SetChannel(4, reinterpret_cast<DWORD>(m_pArea));
    m_fireSoundRef = arrivalSoundRef;

    m_sequenceDelay = sequenceDelay;
    BYTE sequenceLength = m_vidCell.GetSequenceLength(2, FALSE);
    if (sequenceLength < m_sequenceDelay) {
        m_vidCell.SequenceSet(0);
        m_sequenceDelay -= static_cast<BYTE>(m_vidCell.GetSequenceLength(2, FALSE));
    } else {
        m_vidCell.SequenceSet(2);
        m_sequenceDelay = 0;
    }

    m_initialDelay = initialDelay;
    m_bHasHeight = TRUE;
}

// 0x57CEE0 (virtual)
void CProjectileBAM::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    if (m_initialDelay != 0) {
        m_initialDelay--;
        return;
    }

    if (m_sequenceDelay == 0) {
        if (m_vidCell.GetCurrentSequenceId() == 2) {
            if (m_vidCell.IsEndOfSequence(FALSE)) {
                OnArrival();
                return;
            }

            m_vidCell.FrameAdvance();
        } else {
            m_vidCell.SequenceSet(2);
            m_vidCell.FrameSet(0);
        }
    } else {
        m_sequenceDelay--;
        if (!m_vidCell.IsEndOfSequence(FALSE)) {
            m_vidCell.FrameAdvance();
        } else {
            m_vidCell.SequenceSet(1);
            m_vidCell.FrameSet(0);
        }
    }

    m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
}

// 0x57CFB0 (virtual)
void CProjectileBAM::Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface)
{
    CRect rFX;
    CSize frameSize;
    CRect rGCBounds;
    CPoint newPos;
    CPoint ptReference;
    CInfinity* pInfinity;

    (void)pArea;
    UTIL_ASSERT(pVidMode != NULL);
    if (m_initialDelay != 0) {
        return;
    }
    if (!m_pArea->m_visibility.IsTileVisible(m_pArea->m_visibility.PointToTile(m_pos))) {
        return;
    }

    pInfinity = m_pArea->GetInfinity();
    CRect viewport(pInfinity->nCurrentX, pInfinity->nCurrentY,
        pInfinity->nCurrentX + pInfinity->rViewPort.Width(),
        pInfinity->nCurrentY + pInfinity->rViewPort.Height());

    m_vidCell.GetCurrentCenterPoint(ptReference, FALSE);
    m_vidCell.GetCurrentFrameSize(frameSize, FALSE);

    rFX.SetRect(0, 0, frameSize.cx, frameSize.cy);

    newPos.x = m_pos.x;
    newPos.y = m_pArea->GetHeightOffset(m_pos, m_listType) + m_pos.y - m_posZ;

    DWORD dwPrepFlags;
    if (m_visualEffect.m_dwFlags != 0) {
        dwPrepFlags = CInfinity::FXPREP_COPYFROMBACK;
    } else {
        dwPrepFlags = CInfinity::FXPREP_CLEARFILL;
    }

    rGCBounds.left = newPos.x - ptReference.x;
    rGCBounds.top = newPos.y - ptReference.y;
    rGCBounds.right = rGCBounds.left + rFX.Width();
    rGCBounds.bottom = rGCBounds.top + rFX.Height();

    CRect intersection;
    intersection.IntersectRect(rGCBounds, viewport);
    if (intersection.IsRectEmpty()) {
        return;
    }

    pInfinity->FXPrep(rFX,
        dwPrepFlags,
        nSurface,
        newPos,
        ptReference);

    if (pInfinity->FXLock(rFX, dwPrepFlags)) {
        pInfinity->FXRender(&m_vidCell,
            ptReference.x,
            ptReference.y,
            m_visualEffect.m_dwFlags,
            m_visualEffect.m_nTransValue);

        pInfinity->FXRenderClippingPolys(newPos.x,
            newPos.y,
            0,
            ptReference,
            rGCBounds,
            FALSE,
            dwPrepFlags);

        pInfinity->FXUnlock(dwPrepFlags, NULL, CPoint(0, 0));
        pInfinity->FXBltFrom(nSurface,
            rFX,
            newPos.x,
            newPos.y,
            ptReference.x,
            ptReference.y,
            m_visualEffect.m_dwFlags | 0x1);
    }
}

// 0x57D230 (virtual)
void CProjectileBAM::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    (void)targetPos;
    (void)nType;
    m_sourceId = source;
    m_targetId = target;
    m_pArea = pArea;

    CGameObject* pTarget;
    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(target,
        CGameObjectArray::THREAD_ASYNCH, &pTarget, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }
    CPoint liveTarget = pTarget->GetPos();
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(target,
        CGameObjectArray::THREAD_ASYNCH, INFINITE);

    CPoint sourcePos;
    GetStart(source, sourcePos, TRUE);
    m_sound.Play(sourcePos.x, sourcePos.y, 0, FALSE);
    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }
    CGameObject::AddToArea(pArea, sourcePos, nHeight, LIST_FRONT);
    m_targetX = liveTarget.x;
    m_targetY = liveTarget.y;
    m_posAccumX = sourcePos.x << 10;
    m_posAccumY = (sourcePos.y << 12) / 3;
    DeliverEffects();
}

// -----------------------------------------------------------------------------

// 0x57E490
CProjectileSummonVFX::CProjectileSummonVFX(const CResRef& visualResRef, const IcewindCVisualEffect& visualEffect)
    : m_visualEffect(visualEffect)
{
    m_projectileType = 0;
    m_sourceId = CGameObjectArray::INVALID_INDEX;
    m_targetId = CGameObjectArray::INVALID_INDEX;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    m_pArea = NULL;
    m_arrivalSoundRef = "";
    m_loopArrivalSound = FALSE;
    m_bHasHeight = FALSE;
    m_nTargetId = CGameObjectArray::INVALID_INDEX;
    m_offsetAboveTarget = FALSE;

    m_vidCell.SetResRef(visualResRef, FALSE, TRUE, TRUE);
    m_vidCell.SequenceSet(0);
    m_vidCell.FrameSet(0);
}

// 0x57E580 (virtual)
void CProjectileSummonVFX::AIUpdate()
{
    if (m_vidCell.IsEndOfSequence(FALSE)) {
        OnArrival();
        return;
    }

    m_vidCell.FrameAdvance();
    m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
}

// Models the inherited family Render (0x578480, recovered as
// IcewindCProjectileTravellingVFX::Render) under this class's divergent
// local layout; in the binary CProjectileSummonVFX is a family child
// (vtable 0x851234 slot 19 = 0x578480) -- re-parenting is pending.
void CProjectileSummonVFX::Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface)
{
    CRect rFX;
    CSize frameSize;
    CRect rGCBounds;
    CPoint newPos;
    CPoint ptReference;
    CInfinity* pInfinity;

    (void)pVidMode;

    if (pArea == NULL) {
        return;
    }

    pInfinity = pArea->GetInfinity();

    m_vidCell.GetCurrentCenterPoint(ptReference, FALSE);
    m_vidCell.GetCurrentFrameSize(frameSize, FALSE);

    rFX.SetRect(0, 0, frameSize.cx, frameSize.cy);

    newPos.x = m_pos.x;
    newPos.y = pArea->GetHeightOffset(m_pos, m_listType) + m_pos.y - m_posZ;

    DWORD dwPrepFlags;
    if (m_visualEffect.m_dwFlags != 0) {
        dwPrepFlags = CInfinity::FXPREP_COPYFROMBACK;
    } else {
        dwPrepFlags = CInfinity::FXPREP_CLEARFILL;
    }

    rGCBounds.left = newPos.x - ptReference.x;
    rGCBounds.top = newPos.y - ptReference.y;
    rGCBounds.right = rGCBounds.left + rFX.Width();
    rGCBounds.bottom = rGCBounds.top + rFX.Height();

    pInfinity->FXPrep(rFX,
        dwPrepFlags,
        nSurface,
        newPos,
        ptReference);

    if (pInfinity->FXLock(rFX, dwPrepFlags)) {
        pInfinity->FXRender(&m_vidCell,
            ptReference.x,
            ptReference.y,
            m_visualEffect.m_dwFlags,
            m_visualEffect.m_nTransValue);

        pInfinity->FXRenderClippingPolys(newPos.x,
            newPos.y,
            0,
            ptReference,
            rGCBounds,
            FALSE,
            dwPrepFlags);

        pInfinity->FXUnlock(dwPrepFlags, NULL, CPoint(0, 0));
        pInfinity->FXBltFrom(nSurface,
            rFX,
            newPos.x,
            newPos.y,
            ptReference.x,
            ptReference.y,
            m_visualEffect.m_dwFlags | 0x1);
    }
}

// 0x57E710 (virtual)
void CProjectileSummonVFX::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    BYTE rc;

    (void)nHeight;
    (void)nType;

    m_sourceId = source;
    m_targetId = target;
    m_pArea = pArea;
    m_pos.x = targetPos.x;
    m_pos.y = m_offsetAboveTarget ? targetPos.y - 100 : targetPos.y + 1;
    m_posZ = 0;

    // Per-cast visual variety (binary 0x57E776): randomize the overlay vidcell's
    // starting sequence.  The constructor hardcodes the randomize flag (+0x2b8)
    // to 1 (0x57E50A), so the pick is unconditional -- when the BAM holds more
    // than one sequence choose one at random, else sequence 0.  Overrides the
    // constructor's fixed SequenceSet(0) so repeated hits don't always show the
    // same variant.
    SHORT nSeqCount = m_vidCell.GetNumberSequences(FALSE) & 0xFF;
    m_vidCell.SequenceSet(nSeqCount > 1 ? static_cast<SHORT>(rand() % nSeqCount) : 0);

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        CGameObject::AddToArea(pArea, m_pos, 0, CGAMEOBJECT_LIST_FRONT);
        // Impact cue (binary 0x57E838): the overlay plays its fire-sound on launch
        // -- the spell-hit sound (e.g. EFF_M06 for an Invocation hit such as Magic
        // Missile).  The original passes the loop flag at +0x15a (one-shot for these
        // cues, not modelled here) and fireAndForget FALSE.
        PlaySound(m_fireSoundRef, FALSE, FALSE);
        DeliverEffects();
    } else {
        delete this;
    }
}

void CProjectileSummonVFX::SetArrivalSound(const CResRef& arrivalSoundRef)
{
    m_arrivalSoundRef = arrivalSoundRef;
}

void CProjectileSummonVFX::SetOffsetAboveTarget(BOOL offsetAboveTarget)
{
    m_offsetAboveTarget = offsetAboveTarget;
}

// 0x560310
//
// Sub-factory for the casting/"spell hit" overlay projectiles -- the
// DecodeProjectile path for projectileType > 0x1000 (typeIndex =
// projectileType - 0x1001, bPositive selects the good/evil sound variant).
// 107 of the 112 cases build a CProjectileSummonVFX overlay (resref "<spell>H"
// or "<x>X") with per-case copy-from-back / tint-from-flags / caster color-glow
// flash / fire sound / offset-above-target.
//
// Not yet recovered (faithfully omitted, documented per case): the aura-attach
// config on a few overlays, and 4 exotic projectile classes (CallLightning x3
// at 0x5348C0, Sunray at 0x57E860). The color-glow is applied via the caster's
// AddEffect (vtbl+0x78) exactly as the original; AddEffect's own full immunity
// filter (0x733050) is a separate arc.
CProjectile* CProjectileSummonVFX::DecodeSpellHitProjectile(int typeIndex, CGameAIBase* pCaster, BOOL bPositive)
{
    CProjectileSummonVFX* p = NULL;
    switch (typeIndex) {
    case 0:  // plain base CProjectile (ctor 0x530790)
        return new CProjectile();
    case 1: {
        p = new CProjectileSummonVFX(CResRef("AbjurH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        if (pCaster != NULL) {
            pCaster->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0xD7, 0x8C, 0x46, 0x1E),
                CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        p->m_fireSoundRef = bPositive ? CResRef("EFF_P01") : CResRef("EFF_M02");
        break;
    }
    case 2: {
        p = new CProjectileSummonVFX(CResRef("AlterH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        if (pCaster != NULL) {
            pCaster->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0x4B, 0xD2, 0xA0, 0x1E),
                CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        p->m_fireSoundRef = bPositive ? CResRef("EFF_P07") : CResRef("EFF_M08");
        break;
    }
    case 3: {
        p = new CProjectileSummonVFX(CResRef("InvocH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        if (pCaster != NULL) {
            pCaster->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0x6B, 0x06, 0xC9, 0x1E),
                CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        p->m_fireSoundRef = bPositive ? CResRef("EFF_P05") : CResRef("EFF_M06");
        break;
    }
    case 4: {
        p = new CProjectileSummonVFX(CResRef("NecroH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        if (pCaster != NULL) {
            pCaster->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0xB4, 0xD2, 0x50, 0x1E),
                CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        p->m_fireSoundRef = bPositive ? CResRef("EFF_P06") : CResRef("EFF_M07");
        break;
    }
    case 5: {
        p = new CProjectileSummonVFX(CResRef("ConjuH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        if (pCaster != NULL) {
            pCaster->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0x69, 0xD7, 0x46, 0x1E),
                CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        p->m_fireSoundRef = bPositive ? CResRef("EFF_P02") : CResRef("EFF_M03");
        break;
    }
    case 6: {
        p = new CProjectileSummonVFX(CResRef("EnchaH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        if (pCaster != NULL) {
            pCaster->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0x50, 0xD2, 0x50, 0x1E),
                CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        p->m_fireSoundRef = bPositive ? CResRef("EFF_P04") : CResRef("EFF_M05");
        break;
    }
    case 7: {
        p = new CProjectileSummonVFX(CResRef("IllusH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        if (pCaster != NULL) {
            pCaster->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0x50, 0x5A, 0xD2, 0x1E),
                CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        p->m_fireSoundRef = CResRef("EFF_M34");
        break;
    }
    case 8: {
        p = new CProjectileSummonVFX(CResRef("DivinH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        if (pCaster != NULL) {
            pCaster->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0x64, 0x46, 0xD2, 0x1E),
                CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        p->m_fireSoundRef = bPositive ? CResRef("EFF_P03") : CResRef("EFF_M04");
        break;
    }
    case 9: {
        p = new CProjectileSummonVFX(CResRef("ArmorH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M48");
        break;
    }
    case 10: {
        p = new CProjectileSummonVFX(CResRef("SArmorH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M36");
        break;
    }
    case 11: {
        p = new CProjectileSummonVFX(CResRef("GArmorH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M36");
        break;
    }
    case 12: {
        p = new CProjectileSummonVFX(CResRef("StrengH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M12");
        break;
    }
    case 13: {
        p = new CProjectileSummonVFX(CResRef("ConfusH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M16");
        break;
    }
    case 14: {
        p = new CProjectileSummonVFX(CResRef("SOFlamH"), IcewindCVisualEffect());
        break;
    }
    case 15: {
        p = new CProjectileSummonVFX(CResRef("DSpellH"), IcewindCVisualEffect());
        break;
    }
    case 16: {
        p = new CProjectileSummonVFX(CResRef("DisintH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M43");
        break;
    }
    case 17: {
        p = new CProjectileSummonVFX(CResRef("PWSileH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M44");
        break;
    }
    case 18: {
        p = new CProjectileSummonVFX(CResRef("None"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        break;
    }
    case 19: {
        p = new CProjectileSummonVFX(CResRef("FODeatH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M07");
        break;
    }
    case 20: {
        p = new CProjectileSummonVFX(CResRef("MSwordH"), IcewindCVisualEffect());
        break;
    }
    case 21: {
        p = new CProjectileSummonVFX(CResRef("MSumm1H"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        // NOTE: extra aura-attach config (proj +0x2AF/+0x2B0/+0x2B4) omitted.
        break;
    }
    case 22: {
        p = new CProjectileSummonVFX(CResRef("MSumm2H"), IcewindCVisualEffect());
        break;
    }
    case 23: {
        p = new CProjectileSummonVFX(CResRef("MSumm3H"), IcewindCVisualEffect());
        break;
    }
    case 24: {
        p = new CProjectileSummonVFX(CResRef("MSumm4H"), IcewindCVisualEffect());
        break;
    }
    case 25: {
        p = new CProjectileSummonVFX(CResRef("MSumm5H"), IcewindCVisualEffect());
        break;
    }
    case 26: {
        p = new CProjectileSummonVFX(CResRef("MSumm6H"), IcewindCVisualEffect());
        break;
    }
    case 27: {
        p = new CProjectileSummonVFX(CResRef("MSumm7H"), IcewindCVisualEffect());
        break;
    }
    case 28: {
        p = new CProjectileSummonVFX(CResRef("CFElemH"), IcewindCVisualEffect());
        break;
    }
    case 29: {
        p = new CProjectileSummonVFX(CResRef("CEElemH"), IcewindCVisualEffect());
        break;
    }
    case 30: {
        p = new CProjectileSummonVFX(CResRef("CWElemH"), IcewindCVisualEffect());
        break;
    }
    case 31: {
        p = new CProjectileSummonVFX(CResRef("BlessH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P31");
        break;
    }
    case 32: {
        p = new CProjectileSummonVFX(CResRef("CurseH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P32");
        break;
    }
    case 33: {
        p = new CProjectileSummonVFX(CResRef("PrayerH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P31");
        break;
    }
    case 34: {
        p = new CProjectileSummonVFX(CResRef("RecitaH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P44");
        break;
    }
    case 35: {
        p = new CProjectileSummonVFX(CResRef("CLWounH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P26");
        break;
    }
    case 36: {
        p = new CProjectileSummonVFX(CResRef("CMWounH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P26");
        break;
    }
    case 37: {
        p = new CProjectileSummonVFX(CResRef("CSWounH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P34");
        break;
    }
    case 38: {
        p = new CProjectileSummonVFX(CResRef("CCWounH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P46");
        break;
    }
    case 39: {
        p = new CProjectileSummonVFX(CResRef("HealH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P37");
        break;
    }
    case 40: {
        p = new CProjectileSummonVFX(CResRef("ASumm1H"), IcewindCVisualEffect());
        break;
    }
    case 41: {
        p = new CProjectileSummonVFX(CResRef("ASumm2H"), IcewindCVisualEffect());
        break;
    }
    case 42: {
        p = new CProjectileSummonVFX(CResRef("ASumm3H"), IcewindCVisualEffect());
        break;
    }
    case 43: {
        p = new CProjectileSummonVFX(CResRef("SPoisoH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P17");
        break;
    }
    case 44: {
        p = new CProjectileSummonVFX(CResRef("NPoisoH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P41");
        break;
    }
    case 45: {  // Call Lightning (SPPR302): CallLiH beam, sound EFF_P19
        return new CProjectileCallLightning(
            CResRef("CallLiH"), CResRef("EFF_P19"), 0xFF, 1, 0, 0x14);
    }
    case 46: {
        p = new CProjectileSummonVFX(CResRef("SChargH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P42");
        break;
    }
    case 47: {
        p = new CProjectileSummonVFX(CResRef("RParalH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P33");
        break;
    }
    case 48: {
        p = new CProjectileSummonVFX(CResRef("FActioH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P40");
        break;
    }
    case 49: {
        p = new CProjectileSummonVFX(CResRef("MMagicH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P13");
        break;
    }
    case 50: {
        p = new CProjectileSummonVFX(CResRef("SOOneH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P35");
        break;
    }
    case 51: {
        p = new CProjectileSummonVFX(CResRef("CStrenH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M41");
        break;
    }
    case 52: {  // Call Lightning variant: FStrikH beam, sound EFF_P16
        return new CProjectileCallLightning(
            CResRef("FStrikH"), CResRef("EFF_P16"), 0xFF, 1, 0, 0x1C);
    }
    case 53: {
        p = new CProjectileSummonVFX(CResRef("RDeadH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P18");
        break;
    }
    case 54: {
        p = new CProjectileSummonVFX(CResRef("ResurrH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P18");
        break;
    }
    case 55: {
        p = new CProjectileSummonVFX(CResRef("CCommaH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        if (pCaster != NULL) {
            pCaster->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0x80, 0x00, 0x80, 0x1E),
                CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        p->m_fireSoundRef = CResRef("EFF_P47");
        break;
    }
    case 56: {
        p = new CProjectileSummonVFX(CResRef("RWOTFaH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P36");
        break;
    }
    case 57:  // Sunray projectile (class not recovered)
        return NULL;
    case 58: {
        p = new CProjectileSummonVFX(CResRef("SStoneA"), IcewindCVisualEffect());
        p->m_fireSoundRef = CResRef("CRE_P03");
        break;
    }
    case 59: {
        p = new CProjectileSummonVFX(CResRef("DDoorH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M09");
        break;
    }
    case 60: {
        p = new CProjectileSummonVFX(CResRef("DDoorH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M09");
        break;
    }
    case 61: {
        p = new CProjectileSummonVFX(CResRef("CoColdH"), IcewindCVisualEffect());
        break;
    }
    case 62: {
        p = new CProjectileSummonVFX(CResRef("SSOrbH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P38");
        break;
    }
    case 63: {
        p = new CProjectileSummonVFX(CResRef("FireH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        // Fire alone routes its glow through the message queue; the other
        // elemental hits below AddEffect directly.
        if (pCaster != NULL) {
            CMessage* message = new CMessageAddEffect(
                IcewindMisc::CreateEffectColorGlowDissipate(0xFF, 0x00, 0x00, 0x1E),
                pCaster->m_id,
                pCaster->m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        break;
    }
    case 64: {
        p = new CProjectileSummonVFX(CResRef("ColdH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        if (pCaster != NULL) {
            pCaster->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0x00, 0x00, 0xFF, 0x1E),
                CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        break;
    }
    case 65: {
        p = new CProjectileSummonVFX(CResRef("ElectrH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        if (pCaster != NULL) {
            pCaster->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0xFF, 0x4B, 0xFF, 0x1E),
                CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        break;
    }
    case 66: {
        p = new CProjectileSummonVFX(CResRef("AcidH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        if (pCaster != NULL) {
            pCaster->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0x00, 0xFF, 0x00, 0x1E),
                CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        break;
    }
    case 67: {
        p = new CProjectileSummonVFX(CResRef("ParalH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        if (pCaster != NULL) {
            pCaster->AddEffect(IcewindMisc::CreateEffectColorGlowDissipate(0x2D, 0x00, 0x5A, 0x1E),
                CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }
        break;
    }
    case 68: {
        p = new CProjectileSummonVFX(CResRef("MRageH"), IcewindCVisualEffect());
        break;
    }
    case 69: {
        p = new CProjectileSummonVFX(CResRef("RWOTFaG"), IcewindCVisualEffect());
        break;
    }
    case 70: {
        p = new CProjectileSummonVFX(CResRef("BDeath"), IcewindCVisualEffect());
        break;
    }
    case 71: {
        p = new CProjectileSummonVFX(CResRef("PortalH"), IcewindCVisualEffect());
        break;
    }
    case 72: {  // Call Lightning variant: SunscoH beam, sound EFF_P39
        return new CProjectileCallLightning(
            CResRef("SunscoH"), CResRef("EFF_P39"), 0xFF, 1, 0, 0x18);
    }
    case 73: {
        p = new CProjectileSummonVFX(CResRef("BBarrH1"), IcewindCVisualEffect());
        p->m_visualEffect.SetTintFromFlags(TRUE);
        p->m_fireSoundRef = CResRef("ARE_P20");
        // NOTE: extra aura-attach config (proj +0x2AF/+0x2B0/+0x2B4) omitted.
        break;
    }
    case 74: {
        p = new CProjectileSummonVFX(CResRef("BBarrH2"), IcewindCVisualEffect());
        p->m_visualEffect.SetTintFromFlags(TRUE);
        p->SetOffsetAboveTarget(TRUE);
        // NOTE: extra aura-attach config (proj +0x2AF/+0x2B0/+0x2B4) omitted.
        break;
    }
    case 75: {
        p = new CProjectileSummonVFX(CResRef("CoBonH1"), IcewindCVisualEffect());
        p->m_visualEffect.SetTintFromFlags(TRUE);
        p->m_fireSoundRef = CResRef("ARE_P21");
        // NOTE: extra aura-attach config (proj +0x2AF/+0x2B0/+0x2B4) omitted.
        break;
    }
    case 76: {
        p = new CProjectileSummonVFX(CResRef("CoBonH2"), IcewindCVisualEffect());
        p->m_visualEffect.SetTintFromFlags(TRUE);
        p->SetOffsetAboveTarget(TRUE);
        // NOTE: extra aura-attach config (proj +0x2AF/+0x2B0/+0x2B4) omitted.
        break;
    }
    case 77: {
        p = new CProjectileSummonVFX(CResRef("CLDamaH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P103");
        break;
    }
    case 78: {
        p = new CProjectileSummonVFX(CResRef("CMDamaH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P103");
        break;
    }
    case 79: {
        p = new CProjectileSummonVFX(CResRef("CSDamaH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P103");
        break;
    }
    case 80: {
        p = new CProjectileSummonVFX(CResRef("CCDamaH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P103");
        break;
    }
    case 81: {
        p = new CProjectileSummonVFX(CResRef("CDiseaH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P108");
        break;
    }
    case 82: {
        p = new CProjectileSummonVFX(CResRef("PoisonH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P108");
        break;
    }
    case 83: {
        p = new CProjectileSummonVFX(CResRef("SLivinH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P109");
        break;
    }
    case 84: {
        p = new CProjectileSummonVFX(CResRef("HarmH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P103");
        break;
    }
    case 85: {
        p = new CProjectileSummonVFX(CResRef("DestruH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P113");
        break;
    }
    case 86: {
        p = new CProjectileSummonVFX(CResRef("ExaltaH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P106");
        break;
    }
    case 87: {
        p = new CProjectileSummonVFX(CResRef("CloudbH"), IcewindCVisualEffect());
        break;
    }
    case 88: {
        p = new CProjectileSummonVFX(CResRef("MTouchH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P107");
        break;
    }
    case 89: {
        p = new CProjectileSummonVFX(CResRef("MTouchH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P107");
        break;
    }
    case 90: {
        p = new CProjectileSummonVFX(CResRef("CGraceH"), IcewindCVisualEffect());
        break;
    }
    case 91: {
        p = new CProjectileSummonVFX(CResRef("SEaterH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M104");
        // NOTE: extra aura-attach config (proj +0x2AF/+0x2B0/+0x2B4) omitted.
        break;
    }
    case 92: {
        p = new CProjectileSummonVFX(CResRef("SWaveH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_P110");
        break;
    }
    case 93: {
        p = new CProjectileSummonVFX(CResRef("SuffocA"), IcewindCVisualEffect());
        break;
    }
    case 94: {
        p = new CProjectileSummonVFX(CResRef("ADHWilH"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M105");
        break;
    }
    case 95: {
        p = new CProjectileSummonVFX(CResRef("MFMissX"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->m_fireSoundRef = CResRef("EFF_M103");
        break;
    }
    case 96: {
        p = new CProjectileSummonVFX(CResRef("VSpherX"), IcewindCVisualEffect());
        break;
    }
    case 97: {
        p = new CProjectileSummonVFX(CResRef("WVDeatH"), IcewindCVisualEffect());
        break;
    }
    case 98: {
        p = new CProjectileSummonVFX(CResRef("UWardX"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->SetOffsetAboveTarget(TRUE);
        // NOTE: extra aura-attach config (proj +0x2AF/+0x2B0/+0x2B4) omitted.
        break;
    }
    case 99: {
        p = new CProjectileSummonVFX(CResRef("WVHitH"), IcewindCVisualEffect());
        break;
    }
    case 100: {
        p = new CProjectileSummonVFX(CResRef("WDeath1"), IcewindCVisualEffect());
        break;
    }
    case 101: {
        p = new CProjectileSummonVFX(CResRef("WDeath2"), IcewindCVisualEffect());
        break;
    }
    case 102: {
        p = new CProjectileSummonVFX(CResRef("DDeath"), IcewindCVisualEffect());
        break;
    }
    case 103: {
        p = new CProjectileSummonVFX(CResRef("DDeath2"), IcewindCVisualEffect());
        break;
    }
    case 104: {
        p = new CProjectileSummonVFX(CResRef("MSumm1X"), IcewindCVisualEffect());
        break;
    }
    case 105: {
        p = new CProjectileSummonVFX(CResRef("ASumm1X"), IcewindCVisualEffect());
        break;
    }
    case 106: {
        p = new CProjectileSummonVFX(CResRef("CEElemX"), IcewindCVisualEffect());
        break;
    }
    case 107: {
        p = new CProjectileSummonVFX(CResRef("CFElemX"), IcewindCVisualEffect());
        break;
    }
    case 108: {
        p = new CProjectileSummonVFX(CResRef("CWElemX"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        p->SetOffsetAboveTarget(TRUE);
        break;
    }
    case 109: {
        p = new CProjectileSummonVFX(CResRef("GELoopX"), IcewindCVisualEffect());
        p->m_visualEffect.SetCopyFromBack(TRUE);
        break;
    }
    case 110: {
        p = new CProjectileSummonVFX(CResRef("DAttacH"), IcewindCVisualEffect());
        p->m_visualEffect.SetTintFromFlags(TRUE);
        p->m_fireSoundRef = CResRef("CRE_P01");
        break;
    }
    case 111: {
        p = new CProjectileSummonVFX(CResRef("WoMoonX"), IcewindCVisualEffect());
        break;
    }
    default:
        return NULL;
    }
    return p;
}

// ---------------------------------------------------------------------------
// CProjectileCallLightning (SPPR302, vtable 0x84E8C0)
// ---------------------------------------------------------------------------

// 0x5348C0
CProjectileCallLightning::CProjectileCallLightning(const CResRef& resRef,
    const CResRef& soundRef, BYTE colorIndex, LONG renderFlag, LONG animFlag,
    SHORT lifetime)
    : m_palette(CVidPalette::TYPE_RANGE)
{
    // CProjectileInstant() / CProjectile() default-construct the base fields
    // (the binary inlines that initialisation here).
    m_vidCell.SetResRef(resRef, FALSE, TRUE, TRUE);

    // Optional palette recolour.  Every shipped Call Lightning variant passes
    // colorIndex 0xFF (no recolour), so this branch never runs; the master
    // palette bitmap source (global 0x8CF6DC chain) is left unrecovered.
    if (colorIndex != (BYTE)0xFF) {
        // m_palette.SetRange(0, colorIndex, <master palette bitmap>);
        // m_vidCell.SetPalette(m_palette);
    }
    m_vidCell.FrameSet(0);

    m_fireSoundRef = soundRef;

    m_renderFlag = renderFlag;
    m_animFlag = animFlag;
    m_lifetime = lifetime;
}

// 0x535100 (slot 27).  Instant-effect Fire — calls DeliverEffects immediately
// (no travel), then registers in the area as a visual marker.
void CProjectileCallLightning::Fire(CGameArea* pArea, LONG source, LONG target,
    CPoint targetPos, LONG nHeight, SHORT nType)
{
    m_sourceId = source;
    m_targetId = target;
    m_pArea = pArea;

    // The binary calls vtable[0x78] = DeliverEffects() immediately (0x535123).
    DeliverEffects();

    // Register in the global object array.
    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE)
        != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }

    // Resolve the launch point: if homing, read the live target position;
    // otherwise use the passed point.  The binary adds +13 to Y (0x535181).
    CPoint ptLaunch;
    if (m_targetId == CGameObjectArray::INVALID_INDEX) {
        ptLaunch = targetPos;
        ptLaunch.y += 13;
    } else {
        CGameObject* pTarget;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
                CGameObjectArray::THREAD_ASYNCH, &pTarget, INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
        if (rc == CGameObjectArray::SUCCESS) {
            ptLaunch = pTarget->GetPos();
            ptLaunch.y += 13;
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
                CGameObjectArray::THREAD_ASYNCH, INFINITE);
        } else {
            ptLaunch = targetPos;
            ptLaunch.y += 13;
        }
    }

    AddToArea(pArea, ptLaunch, 0, CGameObject::LIST_FRONT);

    // Launch sound (0x535213–0x535290).
    if (m_fireSoundRef != "") {
        PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);
    }

    // Arrival / impact sound (0x535295–0x5352E2).
    if (m_arrivalSoundRef != "") {
        PlaySound(m_arrivalSoundRef, m_loopArrivalSound, FALSE);
    }
}

// 0x534C10 (slot 3).  Lifetime countdown + the bolt's own BAM frame animation.
void CProjectileCallLightning::AIUpdate()
{
    // Time Stop gate (0x534C11–0x534C33): frozen unless this bolt belongs to the
    // caster who stopped time.
    CInfGame* pGame = g_pBaldurChitin->m_pObjectGame;
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    // Lifetime countdown (0x534C39–0x534C97).
    if (--m_lifetime == 0) {
        RemoveFromArea();
        if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE)
            == CGameObjectArray::SUCCESS) {
            delete this;
        }
        return;
    }

    // Frame animation (0x534C9D–).  m_animFlag == 0 => sequential advance,
    // reseeding to frame 0 at the end of the sequence; otherwise => random-frame
    // lightning flicker.
    if (m_animFlag == 0) {
        if (!m_vidCell.IsEndOfSequence(FALSE)) {
            m_vidCell.FrameAdvance();
        } else {
            m_vidCell.FrameSet(0);
        }
    } else {
        BYTE sequenceLength = (BYTE)m_vidCell.GetSequenceLength(0, FALSE);
        if (sequenceLength == 0) {
            m_vidCell.FrameSet(0);
        } else {
            m_vidCell.FrameSet(rand() % sequenceLength);
        }
    }

    // Drop the bolt if its target has left the area (0x534CEB–).
    if (m_targetId != CGameObjectArray::INVALID_INDEX) {
        CGameObject* pTarget;
        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(
            m_targetId, CGameObjectArray::THREAD_ASYNCH, &pTarget, INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            RemoveSelf();
            return;
        }
        if (m_pArea != pTarget->m_pArea) {
            RemoveSelf();
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                m_targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            return;
        }
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
            m_targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }

    m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
}

// 0x534DD0 (vtable slot 19 override). The CALLLIH bolt is a multi-sequence BAM:
// each sequence (cycle) is one vertical slice of the lightning, and they must be
// drawn stacked. The single-cell CProjectileBAM::Render would draw only the current
// slice (the "zoomed" look); this walks every sequence, drawing each one its own
// center-point height higher than the last (nBaseY climbs by ptRef.y per slice), so
// the slices stack into the full stroke from the strike point up. Each slice is
// tile-visibility culled and viewport-clipped; the walk stops the moment a slice
// leaves the map top (nBaseY < 0), is off-screen, or is fully clipped.
// Frida-confirmed on the original: SequenceSet 0/1/2 + an FXRender per cycle per frame.
void CProjectileCallLightning::Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface)
{
    (void)pArea;   // the override reads m_pArea, not the passed area

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
    // __LINE__: 9080
    UTIL_ASSERT(pVidMode != NULL);

    // Viewport clip rect in world coords: scroll origin (nCurrentX/Y) plus the
    // screen rect (rViewPort), read from the area's embedded CInfinity.
    CInfinity* pInfinity = m_pArea->GetInfinity();
    CRect rViewport;
    rViewport.left = pInfinity->nCurrentX;
    rViewport.top = pInfinity->nCurrentY;
    rViewport.right = (pInfinity->rViewPort.right - pInfinity->rViewPort.left)
                      + pInfinity->nCurrentX;
    rViewport.bottom = (pInfinity->rViewPort.bottom - pInfinity->rViewPort.top)
                       + pInfinity->nCurrentY;

    // The strike foot: the cell origin lifted by the terrain height, clamped to the
    // bottom of the map. nBaseY climbs one slice at a time; nSavedX/nSavedBaseY keep
    // the foot for the clipping-poly pass.
    LONG nBaseX = m_pos.x;
    LONG nBaseY = m_pArea->GetHeightOffset(m_pos, m_listType) + m_pos.y;
    if (pInfinity->nAreaY <= nBaseY) {
        nBaseY = pInfinity->nAreaY - 1;
    }
    LONG nSavedX = nBaseX;
    LONG nSavedBaseY = nBaseY;

    if (m_vidCell.GetNumberSequences(FALSE) != 0) {
        DWORD flags = 0;
        BYTE seq = 0;
        do {
            if (nBaseY < 0) {
                return;
            }
            SHORT nTile = (SHORT)((SHORT)(nBaseY / 32) * m_pArea->m_visibility.m_nWidth
                                  + (SHORT)(nBaseX / 32));
            if (!m_pArea->m_visibility.IsTileVisible(nTile)) {
                return;
            }

            m_vidCell.SequenceSet(seq);
            CPoint ptRef;
            m_vidCell.GetCurrentCenterPoint(ptRef, FALSE);
            CSize frameSize;
            m_vidCell.GetCurrentFrameSize(frameSize, FALSE);
            CRect rFrame;
            rFrame.SetRect(0, 0, frameSize.cx, frameSize.cy);

            CRect rDraw;
            rDraw.left = nBaseX - ptRef.x;
            rDraw.top = (m_posZ - ptRef.y) + nBaseY;
            rDraw.right = (rFrame.right - rFrame.left) + rDraw.left;
            rDraw.bottom = (rFrame.bottom - rFrame.top) + rDraw.top;

            CRect rClipped;
            rClipped.IntersectRect(&rDraw, &rViewport);
            if (rClipped.IsRectEmpty()) {
                return;
            }

            DWORD cycleFlags;
            if (m_renderFlag == 0) {
                cycleFlags = CInfinity::FXPREP_CLEARFILL | 0x20001;
            } else {
                cycleFlags = CInfinity::FXPREP_COPYFROMBACK | 0x20200;
            }
            flags |= cycleFlags;

            CPoint newPos(nBaseX, nBaseY);
            pInfinity->FXPrep(rFrame, flags, nSurface, newPos, ptRef);
            if (pInfinity->FXLock(rFrame, flags)) {
                pInfinity->FXRender(&m_vidCell, ptRef.x, ptRef.y, flags, 0);
                CRect rGCBounds;
                rGCBounds.left = rDraw.left;
                rGCBounds.top = m_posZ + rDraw.top;
                rGCBounds.right = rDraw.right;
                rGCBounds.bottom = m_posZ + rDraw.bottom;
                pInfinity->FXRenderClippingPolys(nSavedX, m_posZ + nSavedBaseY, -m_posZ,
                    ptRef, rGCBounds, FALSE, flags);
                pInfinity->FXUnlock(flags, NULL, CPoint(0, 0));
                pInfinity->FXBltFrom(nSurface, rFrame, nBaseX, nBaseY,
                    ptRef.x, ptRef.y, flags);
            }

            nBaseY -= ptRef.y;
            seq++;
        } while (seq < (BYTE)m_vidCell.GetNumberSequences(FALSE));
    }
}

// 0x52AD60
//
// CProjectileTravelling -- shared base ctor for the travelling weapon/spell
// projectiles. Builds the heap CVidCell animation cell from the visual resref,
// a range palette and a bitmap, then seeds the travelling state. The leaf
// classes (CProjectileArrow etc.) prepare the resref and call this, then set
// their own vtable and per-projectile configuration.
//
// STEP 1: the CProjectile base sub-object is constructed by its (implicit)
// member ctor; this body initializes the travelling additions. The original
// also zeroes the motion-integrator work fields in the +0x9C..0xC0 "drift gap"
// (see the projectile-factory layout note) -- the subpixel position, per-tick
// step, carry and random-spread band that AimAtPoint reads each tick -- plus
// the +0xE6 render flags; these are modelled by-name on CProjectileTravelling
// and seeded here. The remaining unread gap defaults (+0xC4, +0xD0..0xDC) are
// omitted. m_driftDecay (the carry modulus) is left unwritten by the original; it
// is only consulted when a carry is non-zero, which the zeroed carries prevent,
// so it is zero-seeded here for definedness.
CProjectileTravelling::CProjectileTravelling(const CResRef& resRef)
    : m_palette(CVidPalette::TYPE_RANGE)
{
    m_mirrorMinX = 0;
    m_mirrorMinY = 0;
    m_distLifetime = 0;
    m_lifetime = 0x7FFF;

    m_pVidCell = new CVidCell(resRef, FALSE);

    m_pShadowCell = NULL;
    m_hasShadowCell = 0;
    m_mirror = 0;
    m_visible = 1;
    m_direction = 0;
    m_paletteSwap = 0;
    m_tinted = 0;
    m_useHeightOffset = 0;
    m_velocity = 0x14;
    m_renderFlags = 0x20000;
    // +0x1D8 is left unwritten by the original base ctor (a leaf sets it, e.g.
    // ARARROW = 0x10); default to 1 (non-directional) for definedness so Render
    // skips the directional-sequence pick until a leaf overrides it.
    m_dirCount = 1;

    // Motion-integrator state (AimAtPoint): subpixel position, step, carry and
    // random-spread band, all zero so the path starts straight from the launch.
    m_posAccumX = 0;
    m_posAccumY = 0;
    m_stepX = 0;
    m_stepY = 0;
    m_driftX = 0;
    m_driftY = 0;
    m_jitterMinX = 0;
    m_jitterMinY = 0;
    m_jitterMaxX = 0;
    m_jitterMaxY = 0;
    m_hasDrift = 0;
    m_driftDecay = 0;
    m_targetX = 0;
    m_targetY = 0;
    m_flightDistSq = 0;

    // Default projectile terrain-cost table (.data 0x8A8154): terrain types
    // 0, 10 and 13 are impassable, everything else costs 5.
    m_terrainTable[0] = CPathSearch::COST_IMPASSABLE;
    m_terrainTable[1] = 5;
    m_terrainTable[2] = 5;
    m_terrainTable[3] = 5;
    m_terrainTable[4] = 5;
    m_terrainTable[5] = 5;
    m_terrainTable[6] = 5;
    m_terrainTable[7] = 5;
    m_terrainTable[8] = 5;
    m_terrainTable[9] = 5;
    m_terrainTable[10] = CPathSearch::COST_IMPASSABLE;
    m_terrainTable[11] = 5;
    m_terrainTable[12] = 5;
    m_terrainTable[13] = CPathSearch::COST_IMPASSABLE;
    m_terrainTable[14] = 5;
    m_terrainTable[15] = 5;

    m_nSpellLevel = 0;
    m_sourceId = 0;
    m_targetId = 0;
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    m_nTargetId = CGameObjectArray::INVALID_INDEX;
}

// 0x52B010 (vtable slot 0)
//
// Frees both heap animation cells (main and shadow); the embedded palette and
// bitmap, and the cells' requested resources (released by ~CVidCell), destruct
// automatically.
CProjectileTravelling::~CProjectileTravelling()
{
    delete m_pVidCell;
    delete m_pShadowCell;
}

// Per-tick flight. Field semantics confirmed by a Frida trace of a Magic
// Missile cast on the original IWD2.exe: target (+0xC8/+0xCC) stays constant,
// the position (CGameObject m_pos) closes on it at ~velocity (+0x70) per tick,
// the lifetime (+0x29E) counts down from 0x7FFF, and the trail field (+0xE2)
// stays 0.
//
// Recovered: the Time Stop gate (CInfGame::m_nTimeStop/m_nTimeStopCaster at
// +0x4B40/+0x4B44), the homing branch (live-target position + height interp
// via CalculateFxRect / delta-Z linear step), and the sparkle-trail spawn
// (factory 0x51AE40 — documented stub, dead for Magic Missile).
// 0x52B900 (vtable slot 3 -- AIUpdate)
void CProjectileTravelling::AIUpdate()
{
    // Time Stop gate (0x52B90E–0x52B93B): during Time Stop, only the
    // caster's own projectiles continue to tick.  The check reads
    // CInfGame::m_nTimeStop and m_nTimeStopCaster at +0x4B40/+0x4B44.
    CInfGame* pGame = g_pBaldurChitin->m_pObjectGame;
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    m_pVidCell->FrameAdvance();

    // Arrival: target and position share the same 16-unit x and 12-unit y cell.
    int posX = m_pos.x;
    if (((m_targetX + ((m_targetX >> 31) & 0xF)) >> 4) == ((posX + ((posX >> 31) & 0xF)) >> 4)
        && m_targetY / 12 == m_pos.y / 12) {
        OnArrival();
        return;
    }

    // Arrival within the per-tick travel radius (velocity + 1); y weighted 16/9.
    int dy = m_targetY - m_pos.y;
    int dist = (dy * dy * 16) / 9 + (m_targetX - posX) * (m_targetX - posX);
    int radius = m_velocity + 1;
    if (dist <= radius * radius) {
        OnArrival();
        return;
    }
    if (m_flightDistSq == 0) {
        OnArrival();
        return;
    }

    // Lifetime countdown.
    SHORT life = m_lifetime;
    m_lifetime = life - 1;
    if (life == 0) {
        RemoveSelf();
        return;
    }

    if (m_targetId == CGameObjectArray::INVALID_INDEX) {
        AimAtPoint(m_targetX, m_targetY);
    } else {
        // Homing branch (0x52BBD7–0x52BD07): share the live target, verify
        // it is still in the same area, read its current position, then
        // interpolate the projectile height toward the target and aim.
        CGameObject* pTarget;
        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH, &pTarget, INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            RemoveSelf();
            return;
        }

        // Save the target id before RemoveSelf can delete this (0x52BAC8).
        if (m_pArea != pTarget->GetArea()) {
            LONG targetId = m_targetId;
            RemoveSelf();
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(targetId,
                CGameObjectArray::THREAD_ASYNCH, INFINITE);
            return;
        }

        CPoint ptLive = pTarget->GetPos();

        // Height interpolation. When m_hasDrift is
        // false (plain Magic Missile), linearly step the projectile height
        // toward the target using the stored delta-Z and distance metric.
        // With drift, the binary queries height but restores the old m_posZ.
        if (m_hasDrift == 0) {
            SHORT stepDeltaZ = static_cast<SHORT>(
                (static_cast<int>(m_nDeltaZ) * dist) / m_flightDistSq);
            m_posZ += static_cast<int>(stepDeltaZ - m_nDeltaZLast);
            m_nDeltaZLast = stepDeltaZ;
        } else if (pTarget->GetObjectType() == CGameObject::TYPE_SPRITE) {
            CGameSprite* pTargetSprite = static_cast<CGameSprite*>(pTarget);
            BYTE rcDeny;
            do {
                rcDeny = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
                    CGameObjectArray::THREAD_ASYNCH, &pTarget, INFINITE);
            } while (rcDeny == CGameObjectArray::SHARED);
            if (rcDeny == CGameObjectArray::SUCCESS) {
                CRect rHeight;
                CPoint ptRef;
                pTargetSprite->GetAnimation()->CalculateFxRect(rHeight, ptRef, pTargetSprite->m_posZ);
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
                    CGameObjectArray::THREAD_ASYNCH, INFINITE);
                LONG targetHeight = (rHeight.bottom - ptRef.y) >> 1;
                if (m_posZ != targetHeight) {
                    LONG oldHeight = m_posZ;
                    m_posZ += (m_nDeltaZ < 0) ? 1 : -1;
                    // The original restores EAX at 0x52BC2A after either step.
                    m_posZ = oldHeight;
                }
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH, INFINITE);

        AimAtPoint(ptLive.x, ptLive.y);
    }

    // Sparkle trail: when m_bSparkleTrail is set the original spawns a
    // sub-projectile at the current position through factory 0x51AE40
    // (0x52BD12–0x52BD5E, new(0xCA) + AddToArea).  Disabled for plain
    // Magic Missile (m_bSparkleTrail stays 0); factory left unrecovered.
    // PARTIAL: sparkle-trail factory 0x51AE40 — documented stub.

    m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
}

// Launch recovery remains partial: the trailing object factory at 0x554D20
// still needs recovery. No recovered leaf currently supplies a nonempty
// trailing resource in field_17E.
// 0x52C050
void CProjectileTravelling::Fire(CGameArea* pArea, LONG source, LONG target,
                                 CPoint targetPos, LONG nHeight, SHORT nType)
{
    (void)nHeight;
    (void)nType;

    m_sourceId = source;
    m_targetId = target;
    m_pArea = pArea;

    CPoint ptTarget;
    BOOL bTargetCreature = FALSE;
    CRect rHeight(0, 0, 0x40, 0x40);
    CPoint ptRef(0x20, 0x32);
    if (m_targetId != CGameObjectArray::INVALID_INDEX) {
        CGameObject* pTargetObj;
        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH, &pTargetObj, INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }
        bTargetCreature = pTargetObj->GetObjectType() == CGameObject::TYPE_SPRITE;
        ptTarget = pTargetObj->GetPos();
        if (bTargetCreature) {
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
                    CGameObjectArray::THREAD_ASYNCH, &pTargetObj, INFINITE);
            } while (rc == CGameObjectArray::SHARED);
            if (rc == CGameObjectArray::SUCCESS) {
                CGameSprite* pTargetSprite = static_cast<CGameSprite*>(pTargetObj);
                pTargetSprite->GetAnimation()->CalculateFxRect(rHeight, ptRef, pTargetSprite->m_posZ);
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
                    CGameObjectArray::THREAD_ASYNCH, INFINITE);
            }
        }
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH, INFINITE);
    } else {
        ptTarget = targetPos;
    }

    // Source height and creature flag: the original gates on m_bHasHeight
    // (+0x16A) and, for a creature source, reads the cast height through
    // CGameAnimationType::GetCastHeight (vtable slot 0xC4/4; 0x52C26E-0x52C2E0).
    // Non-creature or !m_bHasHeight → 0 or 0x20 default.
    LONG nLaunchHeight = 0;
    CPoint ptSourceCenter(0, 0);
    BOOL bSourceCreature = FALSE;
    {
        CGameObject* pSourceObj;
        BYTE rcSrc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH, &pSourceObj, INFINITE);
        if (rcSrc != CGameObjectArray::SUCCESS) {
            return;
        }
        if (rcSrc == CGameObjectArray::SUCCESS) {
            if (m_bHasHeight) {
                if (pSourceObj->GetObjectType() == CGameObject::TYPE_SPRITE) {
                    CGameSprite* pSrcSprite = static_cast<CGameSprite*>(pSourceObj);
                    nLaunchHeight = pSrcSprite->GetAnimation()->GetCastHeight();
                } else {
                    nLaunchHeight = 0x20;
                }
            }
            ptSourceCenter = pSourceObj->GetPos();
            bSourceCreature = (pSourceObj->GetObjectType() == CGameObject::TYPE_SPRITE);
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
                CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    }

    // Launch origin from the source (facing-adjusted).
    CPoint ptSource;
    GetStart(m_sourceId, ptSource, TRUE);

    // Near-target snap: if the source is a creature and the launch point lands
    // within ~3 search cells of the target, snap to the source centre (the
    // self-cast / close-range path; 0x52C34B–0x52C360).
    if (bSourceCreature) {
        int cellDX = ptTarget.x / 16 - ptSource.x / 16;
        int cellDY = ptTarget.y / 12 - ptSource.y / 12;
        if (cellDX * cellDX + cellDY * cellDY < 3) {
            ptSource = ptSourceCenter;
        }
    }

    // Flight distance^2 (y weighted 16/9) -- the metric AIUpdate tests for
    // arrival; drives the lifetime.
    int dx = ptTarget.x - ptSource.x;
    int dy = ptTarget.y - ptSource.y;
    m_flightDistSq = dx * dx + (dy * dy * 16) / 9;

    // Distance-based lifetime (only when m_distLifetime is set; Magic Missile leaves
    // it 0, keeping the 0x7FFF default).
    if (m_distLifetime != 0) {
        m_lifetime = static_cast<SHORT>(
            static_cast<int>(sqrt(static_cast<double>(m_flightDistSq))) / m_velocity + 1);
    }

    m_targetX = ptTarget.x;
    m_targetY = ptTarget.y;

    // Trailing VFX object: when field_17E carries a resource name the
    // original creates a trailing game-object through the factory at
    // 0x554D20, stores its id in m_nTargetId, and dispatches a
    // CMessageProjectileTrailingVFX (vtable 0x84D328) — see 0x52C3C9–0x52C56B.
    // (Identical to the IcewindCProjectileTravellingVFX::Fire path; factory is
    // currently a documented stub — all recovered leaves carry field_17E="".)
    if (m_nTargetId == CGameObjectArray::INVALID_INDEX && !field_17E.IsEmpty()) {
        // STUB: m_nTargetId = CreateTrailingObject(pArea, ptSource, nLaunchHeight,
        //       m_targetId, ptTarget, nHeight, nType);
        // → 0x554D20.  Unrecovered; dead path for all recovered leaves.

        CMessageProjectileTrailingVFX* pMsg = new CMessageProjectileTrailingVFX(
            m_targetId, m_flightDistSq, CResRef(field_17E),
            ptSource.x, ptSource.y, nLaunchHeight, nType,
            static_cast<SHORT>(nLaunchHeight));

        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMsg, FALSE);

        // Post-dispatch: mark the trailing object (0x52C504–0x52C56B).
        CGameObject* pTrailingObj;
        BYTE rcMsg;
        do {
            rcMsg = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(
                m_nTargetId, CGameObjectArray::THREAD_ASYNCH, &pTrailingObj, INFINITE);
        } while (rcMsg == CGameObjectArray::SHARED || rcMsg == CGameObjectArray::DENIED);
        if (rcMsg == CGameObjectArray::SUCCESS) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(
                m_nTargetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    }

    // Launch height block (0x52C621-0x52C657): use the animation rect's
    // half-height, with arithmetic shift, and seed both interpolation terms.
    SHORT nDeltaZ = 0;
    if (bTargetCreature) {
        nDeltaZ = static_cast<SHORT>(nLaunchHeight - ((rHeight.bottom - rHeight.top) >> 1));
    } else {
        m_flightDistSq = 1;
    }
    m_nDeltaZ = nDeltaZ;
    m_nDeltaZLast = nDeltaZ;

    // Register in the global object array (assigns m_id), then add to the area
    // at the cast height — the original gates on m_bHasHeight and reads the
    // source creature's animation cast height (GetCastHeight vtable slot 0xC4).
    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE)
        != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }
    CGameObject::AddToArea(pArea, ptSource, nLaunchHeight, 0);

    // Launch block (0x52C6BA-0x52C805): stop the previous cue, then use the projectile's
    // loop flag for a nonempty launch sound (channel 15, positioned).
    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);

    // Subpixel launch position (1/1024 fixed point), seeded from the launch
    // origin. Frida-confirmed exact: X scaled << 10, Y also 4/3 y-scaled (the
    // iso squash) so AimAtPoint's decode reproduces m_pos. Without this the
    // flight would start from (0, 0).
    m_posAccumX = ptSource.x << 10;
    m_posAccumY = (ptSource.y << 12) / 3;

    // Initial facing toward the target, both points 4/3 y-scaled. The original
    // reads m_pos here -- which the preceding AddToArea set to this same launch
    // origin; AddToArea is not yet wired, so aim from ptSource directly.
    CPoint ptStartScaled;
    ptStartScaled.x = ptSource.x;
    ptStartScaled.y = (ptSource.y * 4) / 3;
    CPoint ptTargetScaled;
    ptTargetScaled.x = ptTarget.x;
    ptTargetScaled.y = (ptTarget.y * 4) / 3;
    m_facing = static_cast<SHORT>(CGameSprite::GetDirection(ptStartScaled, ptTargetScaled));
}

// Vtable slot 33 -- the per-tick motion integrator; AIUpdate's "aim".
//
// Ghidra recovers no function at the vtable target; transcribed from capstone
// disassembly (0x323 bytes, ret 8 -> __thiscall(this, int x, int y)). Steps the
// projectile one tick toward world point (x, y): computes the 16-direction
// facing, a velocity-scaled unit step in 1/1024 fixed point, accumulates it into
// the subpixel position, decodes that back to m_pos, and syncs an attached
// object's height.
//
// All work fields live in the CProjectile gap region in the binary
// (+0x9C..0xE0, +0x1DC); shared launch fields are on CProjectile and travelling
// fields on CProjectileTravelling. The y axis is pre-scaled 4/3 to undo the isometric
// squash before the facing/distance maths (CGameSprite::GetDirection then
// applies its own iso ratios). The rand() term (_rand 0x7E8160) spreads the
// step within the [m_jitterMinX, m_jitterMaxX) band when set -- 0 for Magic Missile, so
// the path stays straight. Depends on Fire seeding the subpixel position
// (m_posAccumX/Y).
// 0x52BD20
void CProjectileTravelling::AimAtPoint(int x, int y)
{
    // Facing toward the target; both points y-scaled 4/3 (undo the iso squash).
    CPoint ptStart;
    ptStart.x = m_pos.x;
    ptStart.y = (m_pos.y * 4) / 3;
    CPoint ptTarget;
    ptTarget.x = x;
    ptTarget.y = (y * 4) / 3;
    m_facing = static_cast<SHORT>(CGameSprite::GetDirection(ptStart, ptTarget));

    // Velocity-scaled unit step (1/1024 fixed point).
    int dx = x - m_pos.x;
    int dyScaled = ptTarget.y - ptStart.y;
    int dist = static_cast<int>(
        sqrt(static_cast<double>(dx * dx + dyScaled * dyScaled)) + 0.5);
    if (dist == 0) {
        m_stepX = 1;
        m_stepY = 1;
    } else {
        m_stepX = ((dx << 10) * m_velocity) / dist;
        m_stepY = ((dyScaled << 10) * m_velocity) / dist;
        m_targetX = x;
        m_targetY = y;

        // Fold in the per-tick carry, then a random spread within the band.
        m_stepX += m_driftX;
        m_stepY += m_driftY;
        int spreadX = m_jitterMaxX - m_jitterMinX;
        if (spreadX > 0) {
            m_stepX += m_jitterMinX + rand() % spreadX;
        }
        int spreadY = m_jitterMaxY - m_jitterMinY;
        if (spreadY > 0) {
            m_stepY += m_jitterMinY + rand() % spreadY;
        }
    }

    // Advance the subpixel position by the step.
    m_posAccumX += m_stepX;
    m_posAccumY += m_stepY;

    // Bleed each carry down by one modulus per tick (zero once exhausted).
    if (m_driftX < 0) {
        m_driftX = (m_driftX <= -m_driftDecay) ? m_driftX + m_driftDecay : 0;
    } else if (m_driftX > 0) {
        m_driftX = (m_driftX >= m_driftDecay) ? m_driftX - m_driftDecay : 0;
    }
    if (m_driftY < 0) {
        m_driftY = (m_driftY <= -m_driftDecay) ? m_driftY + m_driftDecay : 0;
    } else if (m_driftY > 0) {
        m_driftY = (m_driftY >= m_driftDecay) ? m_driftY - m_driftDecay : 0;
    }

    // Decode the subpixel position back to the cell position (undo the 1/1024
    // fixed point and, for y, the 4/3 iso scale).
    m_pos.x = m_posAccumX >> 10;
    m_pos.y = ((m_posAccumY * 3) / 4) >> 10;

    // Keep the attached object (m_nTargetId) at the projectile's height.
    if (m_nTargetId != CGameObjectArray::INVALID_INDEX) {
        CGameObject* pObj;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_nTargetId,
                CGameObjectArray::THREAD_ASYNCH, &pObj, INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }
        pObj->m_posZ = m_posZ;
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_nTargetId,
            CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }
}

// 0x5297D0 (vtable slot 32 -- base blit flags)
//
// Trivial getter: `mov eax, [ecx+0xe6]; ret`. Returns the render-flags field
// the ctor seeds to 0x20000. (A Magic Missile render trace observed 0x20008 --
// the extra 0x8 is OR'd into the field elsewhere on the launch path, not by
// this getter.)
DWORD CProjectileTravelling::GetRenderFlags()
{
    return m_renderFlags;
}

// 0x52C8C0
//
// Picks the cell's animation sequence from the current movement facing, for
// directional projectiles (arrows: m_dirCount == 16). The 16 facings fold onto
// 5 mirrored sequences (the mirror flag is applied separately in Render); an
// 8-direction cell folds onto 3. m_direction caches the last facing the main
// cell was sequenced to, so the sequence is only reset when the facing changes.
// pCell defaults to the main cell; callers pass the shadow cell to sequence it
// without disturbing m_direction.
void CProjectileTravelling::UpdateDirectionSequence(CVidCell* pCell)
{
    if (pCell == NULL) {
        pCell = m_pVidCell;
    }

    SHORT dirCount = m_dirCount;
    if (dirCount == 1) {
        return;
    }
    SHORT facing = m_facing;
    if (m_direction == facing) {
        return;
    }

    if (dirCount == 0x10) {
        switch (facing) {
        case 0: case 8:
            pCell->SequenceSet(0);
            break;
        case 1: case 7: case 9: case 0xF:
            pCell->SequenceSet(1);
            break;
        case 2: case 6: case 0xA: case 0xE:
            pCell->SequenceSet(2);
            break;
        case 3: case 5: case 0xB: case 0xD:
            pCell->SequenceSet(3);
            break;
        case 4: case 0xC:
            pCell->SequenceSet(4);
            break;
        default:
            UTIL_ASSERT(FALSE);
        }
    } else if (dirCount == 8) {
        switch ((facing / 2) * 2) {
        case 0: case 8:
            pCell->SequenceSet(0);
            break;
        case 2: case 6: case 0xA: case 0xE:
            pCell->SequenceSet(2);
            break;
        case 4: case 0xC:
            pCell->SequenceSet(4);
            break;
        default:
            UTIL_ASSERT(FALSE);
        }
    }

    if (pCell == m_pVidCell) {
        m_direction = facing;
    }
}

// 0x52B6B0
//
// Cell draw rect + reference point, mirror/shadow aware: the cell alone,
// the union with the ground shadow cell (the flying cell lifted by m_posZ),
// or the mirror-clamped bounds (flipped about the frame for the north-window
// facings).
void CProjectileTravelling::GetCellBounds(CRect& rBounds, CPoint& ptRef, CVidCell* pCell)
{
    CPoint center;
    CSize size;

    if (pCell == NULL) {
        pCell = m_pVidCell;
    }

    if (m_mirror == 0 && m_hasShadowCell == 0) {
        pCell->GetCurrentCenterPoint(ptRef, FALSE);
        pCell->GetCurrentFrameSize(size, FALSE);
        rBounds.SetRect(0, 0, size.cx, size.cy);
    }

    if (m_hasShadowCell != 0) {
        CPoint shadowCenter;

        pCell->GetCurrentCenterPoint(center, FALSE);
        center.y += m_posZ;
        m_pShadowCell->GetCurrentCenterPoint(shadowCenter, FALSE);

        ptRef.x = center.x;
        ptRef.y = center.y;
        if (ptRef.x < shadowCenter.x) {
            ptRef.x = shadowCenter.x;
        }
        if (ptRef.y < shadowCenter.y) {
            ptRef.y = shadowCenter.y;
        }

        pCell->GetCurrentFrameSize(size, FALSE);
        rBounds.SetRect(0,
            0,
            size.cx + (ptRef.x - center.x),
            size.cy + (ptRef.y - center.y));

        m_pShadowCell->GetCurrentFrameSize(size, FALSE);
        if (rBounds.right < size.cx + (ptRef.x - shadowCenter.x)) {
            rBounds.right = size.cx + (ptRef.x - shadowCenter.x);
        }
        if (rBounds.bottom < size.cy + (ptRef.y - shadowCenter.y)) {
            rBounds.bottom = size.cy + (ptRef.y - shadowCenter.y);
        }
    }

    if (m_mirror != 0) {
        pCell->GetCurrentCenterPoint(center, FALSE);
        pCell->GetCurrentFrameSize(size, FALSE);
        if (CGameSprite::DIR_W < m_direction && m_direction < CGameSprite::DIR_E) {
            center.y = size.cy - center.y;
        }
        center.y += m_posZ;
        int minX = m_mirrorMinX;
        int minY = m_mirrorMinY;
        ptRef.x = center.x;
        ptRef.y = center.y;
        if (ptRef.x < minX) ptRef.x = minX;
        if (ptRef.y < minY) ptRef.y = minY;
        rBounds.SetRect(0, 0, size.cx + (ptRef.x - center.x), size.cy + (ptRef.y - center.y));
        int extX = (ptRef.x - minX) + m_mirrorMinX * 2;
        int extY = m_mirrorMinY * 2 + (ptRef.y - minY);
        if (rBounds.right < extX) rBounds.right = extX;
        if (rBounds.bottom < extY) rBounds.bottom = extY;
    }
}

// 0x531360
CProjectileBolt::CProjectileBolt()
    : CProjectileTravelling(CResRef("BOLT"))
{
    m_pVidCell->SequenceSet(0);

    m_tinted = 1;
    m_useHeightOffset = 0;
    m_mirror = 0;
    m_hasShadowCell = 0;
    m_dirCount = 16;
    m_fireSoundRef = CResRef("");
    m_velocity = static_cast<SHORT>(m_velocity * 5);
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    m_nTargetId = CGameObjectArray::INVALID_INDEX;
}

// 0x5319A0
CProjectileSlingStone::CProjectileSlingStone()
    : CProjectileTravelling(CResRef("MAGICSTN"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 0x1B, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
    m_pVidCell->SetPalette(m_palette);

    m_tinted = 1;
    m_useHeightOffset = 0;
    m_mirror = 0;
    m_hasShadowCell = 0;
    m_dirCount = 1;
    m_fireSoundRef = CResRef("");
    m_velocity = static_cast<SHORT>(m_velocity * 5);
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    m_nTargetId = CGameObjectArray::INVALID_INDEX;
}

// 0x5324A0
//
// CProjectileArrow -- the canonical travelling arrow (DecodeProjectile types
// 0x2/0x5/0x6). Builds the "ARARROW" travelling base, then configures the arrow:
// the first animation sequence, a three-range palette recolour from the game
// master bitmap, a tinted 16-direction cell, and 5x the base velocity.
//
// The original also seeds the fire-sound resref (+0x152) and a +0x17E field;
// both are deferred -- the CResRef members default-construct empty and neither
// is read on the flight/render path. The arrow-specific impact overrides are
// deferred too (see the class comment).
CProjectileArrow::CProjectileArrow()
    : CProjectileTravelling(CResRef("ARARROW"))
{
    m_pVidCell->SequenceSet(0);

    m_palette.SetRange(5, 0x1B, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
    m_palette.SetRange(4, 0x17, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
    m_palette.SetRange(2, 0x2E, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
    m_pVidCell->SetPalette(m_palette);

    m_tinted = 1;
    m_useHeightOffset = 0;
    m_mirror = 0;
    m_hasShadowCell = 0;
    m_dirCount = 0x10;
    m_velocity = static_cast<SHORT>(m_velocity * 5);

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    m_nTargetId = CGameObjectArray::INVALID_INDEX;
}

// 0x531790
//
// CProjectileDart -- the thrown dart. Builds the "DART" travelling base, then
// configures a tinted unmirrored 16-direction missile at 3x the base
// velocity with an empty fire-sound resref.
CProjectileDart::CProjectileDart()
    : CProjectileTravelling(CResRef("DART"))
{
    m_pVidCell->SequenceSet(0);

    m_tinted = 1;
    m_useHeightOffset = 0;
    m_mirror = 0;
    m_hasShadowCell = 0;
    m_dirCount = 0x10;
    m_fireSoundRef = CResRef("");
    m_velocity = static_cast<SHORT>(m_velocity * 3);

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    m_nTargetId = CGameObjectArray::INVALID_INDEX;
}

// 0x532720
//
// CProjectileSPFLMARR -- the generic flame-trailed weapon missile. Builds the
// "SPFLMARR" travelling base, then configures it: first animation sequence, a
// two-range palette recolour (0x43/0x2E) from the game master bitmap, an
// untinted unmirrored 16-direction cell at 5x the base velocity, and the
// default 'A' trail colour ranges (the tinted DecodeProjectile cases restamp
// them).
//
// As with the other leaves, the empty fire-sound resref (+0x152) and +0x17E
// field the original seeds are deferred (both default-construct empty).
CProjectileSPFLMARR::CProjectileSPFLMARR()
    : CProjectileTravelling(CResRef("SPFLMARR"))
{
    m_pVidCell->SequenceSet(0);

    m_palette.SetRange(0, 0x43, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
    m_palette.SetRange(1, 0x2E, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
    m_pVidCell->SetPalette(m_palette);

    m_tinted = 0;
    m_useHeightOffset = 0;
    m_mirror = 0;
    m_hasShadowCell = 0;
    m_dirCount = 0x10;
    m_velocity = static_cast<SHORT>(m_velocity * 5);

    m_trailTick = 0;
    memset(m_trailColorRanges, 0x41, sizeof(m_trailColorRanges));

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    m_nTargetId = CGameObjectArray::INVALID_INDEX;
}

// 0x5329A0
void CProjectileSPFLMARR::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    CString sSoundName("");

    m_trailTick++;
    if (m_trailTick == 1) {
        m_trailTick = 0;

        int nJitterY = rand() % 5;
        int nJitterX = rand() % 5;
        new CGameTemporal(0x301,
            m_trailColorRanges,
            sSoundName,
            m_pArea,
            CPoint(((m_pos.x + nJitterX - 2) << CGameSprite::EXACT_SCALE) - m_stepX * 4 / 3,
                ((m_pos.y + nJitterY - 2) << CGameSprite::EXACT_SCALE) - m_stepY * 4 / 3),
            -m_posZ,
            CPoint(0, 0),
            0,
            0,
            CGameTemporal::COLLISION_DESTROY);

        nJitterY = rand() % 5;
        nJitterX = rand() % 5;
        new CGameTemporal(0x301,
            m_trailColorRanges,
            sSoundName,
            m_pArea,
            CPoint(((m_pos.x + nJitterX - 2) << CGameSprite::EXACT_SCALE) - m_stepX,
                ((m_pos.y + nJitterY - 2) << CGameSprite::EXACT_SCALE) - m_stepY),
            -m_posZ,
            CPoint(0, 0),
            0,
            0,
            CGameTemporal::COLLISION_DESTROY);

        nJitterY = rand() % 5;
        nJitterX = rand() % 5;
        new CGameTemporal(0x301,
            m_trailColorRanges,
            sSoundName,
            m_pArea,
            CPoint(((m_pos.x + nJitterX - 2) << CGameSprite::EXACT_SCALE) - m_stepX * 2 / 3,
                ((m_pos.y + nJitterY - 2) << CGameSprite::EXACT_SCALE) - m_stepY * 2 / 3),
            -m_posZ,
            CPoint(0, 0),
            0,
            0,
            CGameTemporal::COLLISION_DESTROY);
    }

    CProjectileTravelling::AIUpdate();
}

// 0x52CCE0
//
// CProjectileExploding -- the exploding weapon missile base. Builds the
// travelling base on the caller's cell, then arms the strike machinery: one
// strike at everyone of any type inside range 256, re-checked every 100 linger
// ticks. The two explosion cells stay empty here (the deferred leaf Fire and
// Explode paths load them).
CProjectileExploding::CProjectileExploding(const CResRef& resRef)
    : CProjectileTravelling(resRef)
    , m_targetType(0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0)
{
    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    m_nTargetId = CGameObjectArray::INVALID_INDEX;

    m_childProjectileType = 0x4E;
    m_strikesLeft = 1;
    m_nState = 0;
    m_lingerPeriod = 100;
    m_lingerCountdown = 0;
    m_targetType.Set(CAIObjectType::ANYONE);
    m_strikeRange = 0x100;
    m_preCheckRange = 0x100;
    m_bPreScan = 0;
    m_bBurstPending = 0;
    m_tinted = 0;
    m_bExplodeCell1Active = 0;
    m_bExplodeCell2Active = 0;
    m_bCheckNonSprites = 0;
    m_burstType = 0xFF;
}

// 0x52DD60 (vtable slot 3)
//
// The base flying/linger tick -- the leaves override it with their
// flame-trail variants. Flying: advance the cell, re-aim at the recorded
// target point and track the sound. Lingering: animate the explosion cells,
// run the strike pass every m_lingerPeriod ticks, and while charges remain
// fire the pending delayed burst (one-shot) with the colour ranges matching
// m_burstType and a hold of lingerPeriod * strikesLeft.
void CProjectileExploding::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    if (m_nState == 0) {
        m_pVidCell->FrameAdvance();

        LONG nDeltaX = m_targetX - m_pos.x;
        LONG nDeltaY = m_targetY - m_pos.y;
        LONG nRadius = m_velocity + 1;
        if ((nDeltaY * nDeltaY * 16) / 9 + nDeltaX * nDeltaX <= nRadius * nRadius) {
            OnArrival();
            return;
        }

        AimAtPoint(m_targetX, m_targetY);

        // Trailing sub-projectile (m_bSparkleTrail != 0) via the unrecovered
        // factory 0x51AE40 -- omitted like the same documented stub in
        // CProjectileTravelling::AIUpdate. (The original builds the 0xCA
        // object from m_sparkleColor and adds it to the area, LIST_FRONT.)

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
        return;
    }

    if (m_bExplodeCell1Active) {
        m_explodeCell1.FrameAdvance();
    }

    if (m_bExplodeCell2Active) {
        m_explodeCell2.FrameAdvance();
    }

    if (m_lingerCountdown != 0) {
        m_lingerCountdown = m_lingerCountdown - 1;
        return;
    }

    m_lingerCountdown = static_cast<SHORT>(m_lingerPeriod);
    if (AreaEffect(0) != 0) {
        m_strikesLeft = m_strikesLeft - 1;
    }

    if (m_strikesLeft < 1) {
        RemoveFromArea();
        if (g_pBaldurChitin->GetObjectGame()->m_cObjectArray.Delete(m_id,
                CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE)
            == CGameObjectArray::SUCCESS) {
            delete this;
        }

        return;
    }

    if (m_bBurstPending == 0) {
        return;
    }

    BYTE colorRangeValues[7];
    switch (m_burstType) {
    case 0:
        memset(colorRangeValues, 0x43, sizeof(colorRangeValues));
        m_bBurstPending = 0;
        break;
    case 1:
        memset(colorRangeValues, 0x35, sizeof(colorRangeValues));
        m_bBurstPending = 0;
        break;
    case 2:
        memset(colorRangeValues, 0x31, sizeof(colorRangeValues));
        m_bBurstPending = 0;
        break;
    case 3:
        memset(colorRangeValues, 0x47, sizeof(colorRangeValues));
        m_bBurstPending = 0;
        break;
    case 4:
        memset(colorRangeValues, 0x42, sizeof(colorRangeValues));
        m_bBurstPending = 0;
        break;
    case 5:
        memset(colorRangeValues, 0x41, sizeof(colorRangeValues));
        m_bBurstPending = 0;
        break;
    case 0xFF:
        m_bBurstPending = 0;
        return;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
        // __LINE__: 5675
        UTIL_ASSERT(FALSE);
    }

    new CGameFireball3d(m_burstType, colorRangeValues, m_pArea, m_pos, m_strikeRange,
        static_cast<BYTE>(m_velocity), CGameTemporal::COLLISION_DESTROY,
        static_cast<USHORT>(m_lingerPeriod * m_strikesLeft));
}

// 0x78E730 (vtable slot 34; COMDAT-folded with CProjectile::CallBack)
void CProjectileExploding::Explode()
{
}

// 0x52D430
//
// The strike pass (BG2 PDB: CProjectileArea::AreaEffect). Gathers every
// m_targetType creature in m_strikeRange (front and back area lists), fires a
// child projectile of type m_childProjectileType + 1 at each one that passes
// the range/LOS filter, cloning this missile's effect list onto the child,
// and triggers the Explode hook if anything went out. Returns whether the
// pre-scan found targets (always 1 when m_bPreScan is off).
int CProjectileExploding::AreaEffect(BYTE bCheckRange)
{
    CTypedPtrList<CPtrList, LONG*> targets;
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    BOOL bStruck = FALSE;
    int nResult;
    if (m_bPreScan == 0) {
        nResult = 1;
        bStruck = TRUE;
    } else {
        m_pArea->GetCloseObjects(m_posVertList, m_pos, m_targetType, m_preCheckRange,
            m_terrainTable, targets, TRUE, m_bCheckNonSprites);
        m_pArea->GetAllInRangeBack(m_pos, m_targetType, m_preCheckRange,
            m_terrainTable, targets, TRUE, FALSE, m_bCheckNonSprites);
        nResult = targets.GetCount() != 0;
        if (nResult == 0) {
            return nResult;
        }
    }

    targets.RemoveAll();
    m_pArea->GetCloseObjects(m_posVertList, m_pos, m_targetType, m_strikeRange,
        m_terrainTable, targets, TRUE, m_bCheckNonSprites);
    m_pArea->GetAllInRangeBack(m_pos, m_targetType, m_strikeRange,
        m_terrainTable, targets, TRUE, FALSE, m_bCheckNonSprites);

    POSITION pos = targets.GetHeadPosition();
    while (pos != NULL) {
        LONG nTargetId = reinterpret_cast<LONG>(targets.GetNext(pos));

        CGameObject* pObject;
        if (pGame->m_cObjectArray.GetShare(nTargetId, CGameObjectArray::THREAD_ASYNCH,
                &pObject, INFINITE)
            != CGameObjectArray::SUCCESS) {
            return nResult;
        }

        LONG nReleaseId = nTargetId;
        if (pObject->GetObjectType() & CGameObject::TYPE_AIBASE) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);

            CPoint ptTarget;
            if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                ptTarget = pSprite->m_posOld;
            } else {
                ptTarget = pObject->GetPos();
            }

            LONG nDeltaX = ptTarget.x - m_pos.x;
            LONG nDeltaY = ptTarget.y - m_pos.y;
            if (bCheckRange == 0
                || (nDeltaY * nDeltaY * 16) / 9 + nDeltaX * nDeltaX < m_strikeRange * m_strikeRange
                || m_pArea->CheckLOS(m_pos, ptTarget, m_terrainTable, FALSE)) {
                if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                    pSprite->m_posOld = pObject->GetPos();
                }

                bStruck = TRUE;
                CProjectile* pChild = DecodeProjectile(
                    static_cast<USHORT>(m_childProjectileType + 1), NULL, 0);
                pChild->m_nSpellLevel = m_nSpellLevel;
                pChild->m_casterResRef = m_casterResRef;
                pChild->m_fireSoundRef = CResRef("");

                POSITION posEffect = m_effectList.GetHeadPosition();
                while (posEffect != NULL) {
                    CGameEffect* pCopy = m_effectList.GetNext(posEffect)->Copy();
                    pCopy->m_slotNum = pChild->m_projectileType;
                    pChild->m_effectList.AddTail(pCopy);
                }

                pChild->Fire(m_pArea, m_id, nTargetId, pObject->GetPos(), 0x32, 0);
            }

            // The original releases m_callBackProjectile here, not the target
            // it shared above -- the struck target's share is leaked.
            nReleaseId = m_callBackProjectile;
        }

        pGame->m_cObjectArray.ReleaseShare(nReleaseId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }

    if (bStruck) {
        Explode();
    }

    return nResult;
}

// 0x52D9F0 (vtable slot 27 -- Fire; the launch)
//
// The exploding missile's launch. Shares the source for the whole call (spin
// on SHARED/DENIED like the source-position helper), resolves the
// facing-adjusted launch origin, registers in the object array and the area
// (launch height = the source sprite's cast height when m_bHasHeight, 0x20
// for a non-sprite source), plays the launch sound (the inlined
// CProjectile::PlaySound, one-shot unless m_loopFireSound), seeds the
// subpixel position and per-tick steps toward the target and the initial
// facing. A zero-distance launch arrives immediately.
void CProjectileExploding::Fire(CGameArea* pArea, LONG source, LONG target,
    CPoint targetPos, LONG nHeight, SHORT nType)
{
    (void)nHeight;
    (void)nType;

    m_sourceId = source;
    m_targetId = target;
    m_pArea = pArea;

    CGameObject* pSource;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH, &pSource, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    CPoint ptSource;
    GetStart(m_sourceId, ptSource, TRUE);

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE)
        != CGameObjectArray::SUCCESS) {
        // The original frees itself first and reads the source id back off
        // the freed object for the release.
        LONG sourceId = m_sourceId;
        delete this;
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(sourceId,
            CGameObjectArray::THREAD_ASYNCH, INFINITE);
        return;
    }

    LONG nLaunchZ;
    if (m_bHasHeight) {
        if (pSource->GetObjectType() == CGameObject::TYPE_SPRITE) {
            nLaunchZ = static_cast<CGameSprite*>(pSource)->GetAnimation()->GetCastHeight();
        } else {
            nLaunchZ = 0x20;
        }
    } else {
        nLaunchZ = 0;
    }

    AddToArea(pArea, ptSource, nLaunchZ, CGameObject::LIST_FRONT);

    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);

    m_targetX = targetPos.x;
    m_targetY = targetPos.y;
    m_pos.x = ptSource.x;
    m_pos.y = ptSource.y;
    m_posAccumX = ptSource.x << CParticle::RESOLUTION_INC;
    m_posAccumY = ((ptSource.y << CParticle::RESOLUTION_INC) * 4) / 3;

    LONG nDeltaX = m_targetX - m_pos.x;
    LONG nDeltaY = (m_targetY * 4) / 3 - (m_pos.y * 4) / 3;
    LONG nDist = static_cast<LONG>(
        sqrt(static_cast<double>(nDeltaX * nDeltaX + nDeltaY * nDeltaY)) + 0.5);
    if (nDist == 0) {
        OnArrival();
    } else {
        m_stepX = (nDeltaX << CParticle::RESOLUTION_INC) * m_velocity / nDist;
        m_stepY = (nDeltaY << CParticle::RESOLUTION_INC) * m_velocity / nDist;

        CPoint ptStartScaled(m_pos.x, (m_pos.y * 4) / 3);
        CPoint ptTargetScaled(m_targetX, (m_targetY * 4) / 3);
        m_facing = CGameSprite::GetDirection(ptStartScaled, ptTargetScaled);
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH, INFINITE);
}

// 0x52D7F0 (vtable slot 28 -- OnArrival)
//
// Arrival: notify the callback projectile (its CallBack virtual, under an
// exclusive deny grab), switch to the linger state so AIUpdate runs the
// strike pass, hide the missile and play the arrival sound (the inlined
// CProjectile::PlaySound, fire-and-forget).
void CProjectileExploding::OnArrival()
{
    if (m_callBackProjectile != CGameObjectArray::INVALID_INDEX) {
        CGameObject* pObject;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(
                m_callBackProjectile, CGameObjectArray::THREAD_ASYNCH, &pObject, INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        static_cast<CProjectile*>(pObject)->CallBack();
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_callBackProjectile,
            CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }

    m_nState = 1;
    m_visible = 0;
    PlaySound(m_arrivalSoundRef, m_loopArrivalSound, TRUE);
}

// 0x52E9F0
//
// CProjectileExplodingFlame -- the exploding flame missile. Builds the
// "SPFLMARR" exploding base, then configures it like its plain sibling
// CProjectileSPFLMARR: first animation sequence, a two-range palette recolour
// (0x43/0x39, ranges 0 and 2) from the game master bitmap, an untinted
// unmirrored 16-direction cell at 3x the base velocity, an empty fire-sound
// resref, and the default 'A' trail colour ranges.
CProjectileExplodingFlame::CProjectileExplodingFlame()
    : CProjectileExploding(CResRef("SPFLMARR"))
{
    m_pVidCell->SequenceSet(0);

    m_palette.SetRange(0, 0x43, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
    m_palette.SetRange(2, 0x39, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
    m_pVidCell->SetPalette(m_palette);

    m_velocity = static_cast<SHORT>(m_velocity * 3);
    m_tinted = 0;
    m_useHeightOffset = 0;
    m_mirror = 0;
    m_hasShadowCell = 0;
    m_dirCount = 0x10;
    m_fireSoundRef = CResRef("");

    memset(m_trailColorRanges, 0x41, sizeof(m_trailColorRanges));
    m_trailTick = 0;
    m_childProjectileType = 0x4E;
}

// 0x52EC80
void CProjectileExplodingFlame::AIUpdate()
{
    CString sSoundName("");

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
        if (m_nState == 0) {
            // Flying: advance the cell, chase the (live) target, drop the
            // flame trail.
            m_pVidCell->FrameAdvance();

            LONG nDeltaX = m_targetX - m_pos.x;
            LONG nDeltaY = m_targetY - m_pos.y;
            LONG nRadius = m_velocity + 1;
            if (nRadius * nRadius < (nDeltaY * nDeltaY * 16) / 9 + nDeltaX * nDeltaX) {
                if (m_targetId == CGameObjectArray::INVALID_INDEX) {
                    AimAtPoint(m_targetX, m_targetY);
                } else {
                    CGameObject* pTarget;
                    if (pGame->m_cObjectArray.GetShare(m_targetId,
                            CGameObjectArray::THREAD_ASYNCH, &pTarget, INFINITE)
                        != CGameObjectArray::SUCCESS) {
                        RemoveSelf();
                        return;
                    }

                    if (m_pArea != pTarget->m_pArea) {
                        LONG nTargetId = m_targetId;
                        RemoveSelf();
                        pGame->m_cObjectArray.ReleaseShare(nTargetId,
                            CGameObjectArray::THREAD_ASYNCH, INFINITE);
                        return;
                    }

                    CPoint ptTarget = pTarget->GetPos();
                    pGame->m_cObjectArray.ReleaseShare(m_targetId,
                        CGameObjectArray::THREAD_ASYNCH, INFINITE);
                    AimAtPoint(ptTarget.x, ptTarget.y);
                }

                m_trailTick = m_trailTick + 1;
                if (m_trailTick == 1) {
                    m_trailTick = 0;

                    int nJitterY = rand() % 5;
                    int nJitterX = rand() % 5;
                    new CGameTemporal(0x300,
                        m_trailColorRanges,
                        sSoundName,
                        m_pArea,
                        CPoint(((m_pos.x + nJitterX - 2) << CGameSprite::EXACT_SCALE) - m_stepX * 4 / 3,
                            ((m_pos.y + nJitterY - 2) << CGameSprite::EXACT_SCALE) - m_stepY * 4 / 3),
                        -m_posZ,
                        CPoint(0, 0),
                        0,
                        0,
                        CGameTemporal::COLLISION_DESTROY);

                    nJitterY = rand() % 5;
                    nJitterX = rand() % 5;
                    new CGameTemporal(0x300,
                        m_trailColorRanges,
                        sSoundName,
                        m_pArea,
                        CPoint(((m_pos.x + nJitterX - 2) << CGameSprite::EXACT_SCALE) - m_stepX,
                            ((m_pos.y + nJitterY - 2) << CGameSprite::EXACT_SCALE) - m_stepY),
                        -m_posZ,
                        CPoint(0, 0),
                        0,
                        0,
                        CGameTemporal::COLLISION_DESTROY);

                    nJitterY = rand() % 5;
                    nJitterX = rand() % 5;
                    new CGameTemporal(0x300,
                        m_trailColorRanges,
                        sSoundName,
                        m_pArea,
                        CPoint(((m_pos.x + nJitterX - 2) << CGameSprite::EXACT_SCALE) - m_stepX * 2 / 3,
                            ((m_pos.y + nJitterY - 2) << CGameSprite::EXACT_SCALE) - m_stepY * 2 / 3),
                        -m_posZ,
                        CPoint(0, 0),
                        0,
                        0,
                        CGameTemporal::COLLISION_DESTROY);
                }

                // Trailing sub-projectile (+0xE2 != 0) via the unrecovered
                // factory 0x51AE40 -- omitted like the same documented stub in
                // CProjectileTravelling::AIUpdate.

                m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
            } else {
                OnArrival();
            }
        } else {
            // Lingering after arrival: run the strike pass every
            // m_lingerPeriod ticks until the charges run out.
            if (m_lingerCountdown == 0) {
                m_lingerCountdown = static_cast<SHORT>(m_lingerPeriod);
                if (AreaEffect(0) != 0) {
                    m_strikesLeft = m_strikesLeft - 1;
                }

                if (m_strikesLeft < 1) {
                    RemoveFromArea();
                    if (pGame->m_cObjectArray.Delete(m_id, CGameObjectArray::THREAD_ASYNCH,
                            NULL, INFINITE)
                        == CGameObjectArray::SUCCESS) {
                        delete this;
                    }

                    return;
                }
            }

            m_lingerCountdown = m_lingerCountdown - 1;
        }
    }
}

// 0x52E230
//
// CProjectileExplodingWeapon -- the exploding thrown-weapon missile. Builds
// the "SPFIREBL" exploding base, then configures a mirrored 16-direction
// missile at 2x the base velocity (the MMissiT mirror geometry), a one-range
// palette recolour (0x43) from the game master bitmap, an empty fire-sound
// resref and the default 'A' trail colour ranges.
CProjectileExplodingWeapon::CProjectileExplodingWeapon()
    : CProjectileExploding(CResRef("SPFIREBL"))
{
    m_pVidCell->SequenceSet(0);

    m_palette.SetRange(0, 0x43, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
    m_pVidCell->SetPalette(m_palette);

    m_mirrorMinX = 0xF;
    m_mirrorMinY = 0xB;
    m_explodeColorRange = 0x43;
    m_tinted = 0;
    m_useHeightOffset = 0;
    m_mirror = 1;
    m_leafRenderParam = 0x80;
    m_hasShadowCell = 0;
    m_dirCount = 0x10;
    m_velocity = static_cast<SHORT>(m_velocity << 1);
    m_fireSoundRef = CResRef("");

    memset(m_trailColorRanges, 0x41, sizeof(m_trailColorRanges));
    m_trailTick = 0;
    m_childProjectileType = 0x4E;
}

// 0x52E4D0
//
// Same tick as CProjectileExplodingFlame::AIUpdate minus the live-target
// homing: the missile only ever re-aims at the recorded target point.
void CProjectileExplodingWeapon::AIUpdate()
{
    CString sSoundName("");

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
        if (m_nState == 0) {
            m_pVidCell->FrameAdvance();

            LONG nDeltaX = m_targetX - m_pos.x;
            LONG nDeltaY = m_targetY - m_pos.y;
            LONG nRadius = m_velocity + 1;
            if (nRadius * nRadius < (nDeltaY * nDeltaY * 16) / 9 + nDeltaX * nDeltaX) {
                AimAtPoint(m_targetX, m_targetY);

                m_trailTick = m_trailTick + 1;
                if (m_trailTick == 1) {
                    m_trailTick = 0;

                    int nJitterY = rand() % 5;
                    int nJitterX = rand() % 5;
                    new CGameTemporal(0x300,
                        m_trailColorRanges,
                        sSoundName,
                        m_pArea,
                        CPoint(((m_pos.x + nJitterX - 2) << CGameSprite::EXACT_SCALE) - m_stepX * 4 / 3,
                            ((m_pos.y + nJitterY - 2) << CGameSprite::EXACT_SCALE) - m_stepY * 4 / 3),
                        -m_posZ,
                        CPoint(0, 0),
                        0,
                        0,
                        CGameTemporal::COLLISION_DESTROY);

                    nJitterY = rand() % 5;
                    nJitterX = rand() % 5;
                    new CGameTemporal(0x300,
                        m_trailColorRanges,
                        sSoundName,
                        m_pArea,
                        CPoint(((m_pos.x + nJitterX - 2) << CGameSprite::EXACT_SCALE) - m_stepX,
                            ((m_pos.y + nJitterY - 2) << CGameSprite::EXACT_SCALE) - m_stepY),
                        -m_posZ,
                        CPoint(0, 0),
                        0,
                        0,
                        CGameTemporal::COLLISION_DESTROY);

                    nJitterY = rand() % 5;
                    nJitterX = rand() % 5;
                    new CGameTemporal(0x300,
                        m_trailColorRanges,
                        sSoundName,
                        m_pArea,
                        CPoint(((m_pos.x + nJitterX - 2) << CGameSprite::EXACT_SCALE) - m_stepX * 2 / 3,
                            ((m_pos.y + nJitterY - 2) << CGameSprite::EXACT_SCALE) - m_stepY * 2 / 3),
                        -m_posZ,
                        CPoint(0, 0),
                        0,
                        0,
                        CGameTemporal::COLLISION_DESTROY);
                }

                // Trailing sub-projectile (+0xE2 != 0) via the unrecovered
                // factory 0x51AE40 -- omitted like the same documented stub in
                // CProjectileTravelling::AIUpdate.

                m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
            } else {
                OnArrival();
            }
        } else {
            if (m_lingerCountdown == 0) {
                m_lingerCountdown = static_cast<SHORT>(m_lingerPeriod);
                if (AreaEffect(0) != 0) {
                    m_strikesLeft = m_strikesLeft - 1;
                }

                if (m_strikesLeft < 1) {
                    RemoveFromArea();
                    if (pGame->m_cObjectArray.Delete(m_id, CGameObjectArray::THREAD_ASYNCH,
                            NULL, INFINITE)
                        == CGameObjectArray::SUCCESS) {
                        delete this;
                    }

                    return;
                }
            }

            m_lingerCountdown = m_lingerCountdown - 1;
        }
    }
}

// 0x52F1C0 (vtable slot 34)
//
// The flame missile's explosion: one fireball burst at the missile position,
// radius = the strike range, expansion speed = half the flight velocity,
// colour ranges hardwired to 0x43.
void CProjectileExplodingFlame::Explode()
{
    BYTE colorRangeValues[7];
    memset(colorRangeValues, 0x43, sizeof(colorRangeValues));
    new CGameFireball3d(CGameFireball3d::TYPE_FIREBALL, colorRangeValues, m_pArea, m_pos,
        m_strikeRange, static_cast<BYTE>(m_velocity) >> 1, CGameTemporal::COLLISION_DESTROY, 0);
}

// 0x52E940 (vtable slot 34)
//
// The weapon missile's explosion: the same fireball burst, coloured by the
// leaf's explosion colour range (the tinted DecodeProjectile cases restamp
// it).
void CProjectileExplodingWeapon::Explode()
{
    BYTE colorRangeValues[7];
    memset(colorRangeValues, m_explodeColorRange, sizeof(colorRangeValues));
    new CGameFireball3d(CGameFireball3d::TYPE_FIREBALL, colorRangeValues, m_pArea, m_pos,
        m_strikeRange, static_cast<BYTE>(m_velocity) >> 1, CGameTemporal::COLLISION_DESTROY, 0);
}

// 0x52F260
//
// CProjectileSkullTrap -- the Skull Trap delayed-explosion projectile
// (DecodeProjectile type 0x60). Builds the CProjectileExploding base from the
// missile BAM, then adds a shadow animation cell from the explosion BAM and
// arms both embedded explosion cells. Flies at double the base velocity. As
// with the other leaves the original's copy of an empty global CResRef into
// m_fireSoundRef and of the nil-string sentinel into m_explodeSound are
// deferred -- both members default-construct empty.
CProjectileSkullTrap::CProjectileSkullTrap(const CResRef& cMissileRef, const CResRef& cExplodeRef, SHORT nType)
    : CProjectileExploding(cMissileRef)
{
    m_pShadowCell = new CVidCell(cExplodeRef, FALSE);
    m_pShadowCell->SequenceSet(0);
    m_pVidCell->SequenceSet(0);

    m_tinted = 0;
    m_useHeightOffset = 0;
    m_mirror = 0;
    m_hasShadowCell = 1;
    m_dirCount = 1;
    m_velocity = static_cast<SHORT>(m_velocity << 1);

    m_nType = nType;
    m_childProjectileType = 0x4E;
    m_strikesLeft = 1;
    m_lingerPeriod = 100;
    m_bPreScan = 1;

    m_explodeCell1.SetResRef(cMissileRef, FALSE, TRUE, TRUE);
    m_bExplodeCell1Active = (cMissileRef != "");

    m_explodeCell2.SetResRef(cExplodeRef, FALSE, TRUE, TRUE);
    m_bExplodeCell2Active = (cExplodeRef != "");
}

// 0x52F9E0 (vtable slot 3)
//
// Skull Trap tick. While flying (m_nState == 0): advance the cell, and once the
// trap reaches its target point fire OnArrival; otherwise re-aim and keep the
// sound positioned. Once armed it advances the explosion cells and, every
// m_lingerPeriod, runs an AreaEffect strike pass -- each landed strike spends a
// charge and re-arms the timer; when the charges run out the trap removes
// itself and is freed.
void CProjectileSkullTrap::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    if (m_nState == 0) {
        m_pVidCell->FrameAdvance();

        LONG nDeltaX = m_targetX - m_pos.x;
        LONG nDeltaY = m_targetY - m_pos.y;
        LONG nRadius = m_velocity + 1;
        if ((nDeltaY * nDeltaY * 16) / 9 + nDeltaX * nDeltaX <= nRadius * nRadius) {
            OnArrival();
            return;
        }

        AimAtPoint(m_targetX, m_targetY);

        // Trailing sub-projectile (m_bSparkleTrail != 0) via the unrecovered
        // factory 0x51AE40 -- omitted like the same documented stub in
        // CProjectileExploding::AIUpdate. (The original builds the 0xCA object
        // from m_sparkleColor and adds it to the area.)

        m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
        return;
    }

    if (m_bExplodeCell1Active) {
        m_explodeCell1.FrameAdvance();
    }

    if (m_bExplodeCell2Active) {
        m_explodeCell2.FrameAdvance();
    }

    if (m_lingerCountdown == 0) {
        if (AreaEffect(0) != 0) {
            m_strikesLeft = m_strikesLeft - 1;
            m_lingerCountdown = static_cast<SHORT>(m_lingerPeriod);
        }

        if (m_strikesLeft < 1) {
            RemoveFromArea();
            if (g_pBaldurChitin->GetObjectGame()->m_cObjectArray.Delete(m_id,
                    CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE)
                == CGameObjectArray::SUCCESS) {
                delete this;
                return;
            }
        }
    }
    else {
        m_lingerCountdown = m_lingerCountdown - 1;
    }
}

// 0x52F760 (vtable slot 34)
//
// Skull Trap detonation: play the explosion sound (m_explodeSound) as a
// fire-and-forget one-shot on the area channel at the trap position.
//
// The visible burst -- (rand() % 16) + 10 explosion temporals spawned from
// m_nType at the trap position with a +/-5 random scatter via the unrecovered
// factory at 0x70F4F0 -- is omitted (as the trail factory at 0x51AE40 is),
// so it cannot be reproduced faithfully yet.
void CProjectileSkullTrap::Explode()
{
    CSound sound;
    sound.SetResRef(CResRef(m_explodeSound), TRUE, TRUE);
    sound.SetFireForget(TRUE);
    sound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
    sound.Play(m_pos.x, m_pos.y, 0, FALSE);
}

// 0x5300E0
//
// CProjectileStrike -- the invisible per-target strike bolt. Builds the
// "SPFIREBL" travelling base, then hides it and arms the distance-derived
// flight lifetime; the strike pass clones the parent's effect list onto it
// before firing, so it is a pure effect carrier.
CProjectileStrike::CProjectileStrike()
    : CProjectileTravelling(CResRef("SPFIREBL"))
{
    m_pVidCell->SequenceSet(0);

    m_palette.SetRange(0, 0x43, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
    m_pVidCell->SetPalette(m_palette);

    m_visible = 0;
    m_dirCount = 0x10;

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    field_17E = "";
    m_nTargetId = CGameObjectArray::INVALID_INDEX;
    m_distLifetime = 1;
}

// 0x57E030
//
// CProjectileMMissiT -- the Magic Missile homing sub-missile (DecodeProjectile
// type 0xDA). Builds the "MMissiT" travelling base, then configures a mirrored
// 16-direction-ish missile at double the base velocity. The factory constructs
// it with nPaletteFlag == 0, so the optional palette recolour is skipped.
//
// As with the other leaves, the fire-sound resref (+0x152) and the +0x17E field
// the original seeds are deferred (empty CResRef default, not read on the
// flight path).
CProjectileMMissiT::CProjectileMMissiT(SHORT nPaletteFlag)
    : CProjectileTravelling(CResRef("MMissiT"))
{
    m_pVidCell->SequenceSet(0);

    m_velocity = static_cast<SHORT>(m_velocity << 1);
    m_mirrorMinX = 0xF;
    m_mirrorMinY = 0xB;
    m_tinted = 0;
    m_useHeightOffset = 0;
    m_mirror = 1;
    m_leafRenderParam = 0x80;
    m_hasShadowCell = 0;
    m_renderFlags |= 8;

    if (nPaletteFlag != 0) {
        if (nPaletteFlag == 1) {
            m_palette.SetRange(0, 0x21, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
            m_pVidCell->SetPalette(m_palette);
        } else {
            UTIL_ASSERT(FALSE);
        }
    }

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    m_nTargetId = CGameObjectArray::INVALID_INDEX;
}

// 0x52CA10
//
// CProjectileSparkle -- the single travelling spell missile, "SPMAGMIS" BAM
// (the Magic Missile bolt; the sparkle-stream factory cases swap the cell to
// "TRAVEL" afterwards). Same shape as CProjectileMMissiT -- a mirrored
// missile at double the base velocity with the optional TYPE_RANGE palette
// recolour -- plus an explicit clearing of the shadow-cell, direction and
// lifetime fields.
//
// As with the other leaves, the original's copies of an empty global CResRef
// into m_fireSoundRef (+0x152) and of an empty 8-byte global into field_17E
// are deferred -- both members default-construct empty.
CProjectileSparkle::CProjectileSparkle(SHORT nPaletteType)
    : CProjectileTravelling(CResRef("SPMAGMIS"))
{
    m_pShadowCell = NULL;
    m_dirCount = 0;
    m_direction = 0;
    m_facing = 0;
    m_visible = 0;
    m_paletteSwap = 0;
    m_distLifetime = 0;
    m_lifetime = 0;

    m_pVidCell->SequenceSet(0);

    m_velocity = static_cast<SHORT>(m_velocity << 1);
    m_mirrorMinX = 0xF;
    m_mirrorMinY = 0xB;
    m_tinted = 0;
    m_useHeightOffset = 0;
    m_mirror = 1;
    m_leafRenderParam = 0x80;
    m_hasShadowCell = 0;
    m_renderFlags |= 8;

    if (nPaletteType != 0) {
        if (nPaletteType == CVidPalette::TYPE_RANGE) {
            m_palette.SetRange(0, 0x21, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
            m_pVidCell->SetPalette(m_palette);
        } else {
            UTIL_ASSERT(FALSE);
        }
    }

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    m_nTargetId = CGameObjectArray::INVALID_INDEX;
}

// 0x5295D0
//
// Replace the travelling animation cell built by the base constructor with
// another BAM (the sparkle cases swap "SPMAGMIS" for the "TRAVEL" dot).
void CProjectileTravelling::SetVidCell(CResRef resRef)
{
    if (m_pVidCell != NULL) {
        delete m_pVidCell;
    }
    m_pVidCell = new CVidCell(resRef, FALSE);
}

// 0x529660
//
// Request the sparkle colour-table bitmap ("STTRAVL1"; resource type 1 =
// bitmap) and arm the render-time palette swap (m_paletteSwap; Render picks
// the row given by m_sparkleColor). Cancels the previous request when the
// resref actually changes; an empty name is ignored. (Name is a #guess.)
void CProjectileTravelling::SetTravelPalette(CString bitmapName)
{
    if (bitmapName.GetLength() != 0) {
        CResRef resRef(bitmapName);
        if (resRef != m_travelPaletteRef) {
            if (m_pTravelPaletteRes != NULL
                && m_travelPaletteRef != CResRef("")
                && m_travelPaletteRequested) {
                m_pTravelPaletteRes->CancelRequest();
            }
            if (resRef == CResRef("")) {
                m_pTravelPaletteRes = NULL;
                m_travelPaletteRef = CResRef("");
            } else {
                CRes* pRes = g_pChitin->cDimm.GetResObject(resRef, 1, TRUE);
                if (pRes != NULL) {
                    m_pTravelPaletteRes = static_cast<CResBitmap*>(pRes);
                    m_travelPaletteRequested = TRUE;
                    pRes->Request();
                    m_travelPaletteRef = resRef;
                } else {
                    m_pTravelPaletteRes = NULL;
                    m_travelPaletteRef = CResRef("");
                }
            }
        }
        field_298 = 0;
        m_paletteSwap = 1;
    }
}

// 0x5309C0
//
// CProjectileMagicMissileMulti -- the Magic Missile launcher base. Builds the
// travelling base, then pre-spawns nCount sub-missiles (DecodeProjectile of
// nSubType + 1) into the sub-missile list; the launcher's Fire later drains them
// into the area. Each sub-missile is born with the "TRA_02" launch sound (stamped
// by its DecodeProjectile case); the original clears it on every sub-missile after
// the first (0x530ACB, assigning the empty default CResRef), so a volley plays a
// single launch whoosh rather than one per missile.
CProjectileMagicMissileMulti::CProjectileMagicMissileMulti(const CResRef& resRef,
    SHORT nCount, USHORT nSubType, BYTE nPaletteFlag)
    : CProjectileTravelling(resRef)
{
    m_pVidCell->SequenceSet(0);
    m_missileCount = nCount;
    m_subType = nSubType;
    m_renderFlags |= 8;

    if (nPaletteFlag != 0) {
        if (nPaletteFlag == 1) {
            m_palette.SetRange(0, 0x21, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
            m_pVidCell->SetPalette(m_palette);
        } else {
            UTIL_ASSERT(FALSE);
        }
    }

    for (SHORT i = 0; i < nCount; ++i) {
        CProjectile* pSub = CProjectile::DecodeProjectile(
            static_cast<USHORT>(m_subType + 1), NULL, 0);
        if (i != 0) {
            pSub->m_fireSoundRef = CResRef();
        }
        m_subMissiles.AddTail(pSub);
    }

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    m_nTargetId = CGameObjectArray::INVALID_INDEX;
}

// 0x531120
//
// CProjectileSPMAGMIS -- the Magic Missile launcher leaf (DecodeProjectile types
// 0x44-0x48). nCount = the band's missile count (1..5); the sub-missiles are
// "MMissiT" (sub-type 0xD9 -> spawned type 0xDA).
CProjectileSPMAGMIS::CProjectileSPMAGMIS(SHORT nCount, SHORT nPaletteFlag)
    : CProjectileMagicMissileMulti(CResRef("SPMAGMIS"), nCount, 0xD9,
        static_cast<BYTE>(nPaletteFlag))
{
    m_tinted = 0;
    m_useHeightOffset = 0;
    m_hasShadowCell = 0;
    m_mirrorMinX = 10;
    m_mirrorMinY = 7;
    m_mirror = 1;
    m_leafRenderParam = 0x80;
    m_dirCount = 1;

    m_callBackProjectile = CGameObjectArray::INVALID_INDEX;
    m_nTargetId = CGameObjectArray::INVALID_INDEX;
}

// 0x530C90 (vtable slot 27 -- Fire; the multi-missile launch)
//
// Recovered from the ~344-instruction Ghidra-empty original (capstone disasm).
// Launches the pre-spawned MMissiT sub-missiles in a fan: it resolves the caster
// (source) and target positions, builds the launch direction (source - target,
// the y un-squashed by 4/3 to undo the iso projection), then walks the
// sub-missile list two at a time and gives each pair an equal-and-opposite
// perpendicular drift (the running spread index times the unit normal). The
// drift seeds the per-tick carry (m_driftX/m_driftY) which AimAtPoint folds into
// the homing step and bleeds off by m_driftDecay each tick, so the missiles splay
// out then curve back onto the shared target -- the classic Magic Missile
// spread. An odd man out flies straight.
//
// Per sub-missile it also clones the launcher's gameplay effects onto the
// missile's own list (so each missile carries the damage), copies the spell
// level/caster resref, and jitters the missile velocity by rand()%20 - 10 for a
// staggered arrival. It then drains the staging list and deletes itself (the
// launcher is never added to the area; CMessageFireProjectile::Run does not
// touch it after Fire, so the self-delete is safe).
//
// NOTE: the cloned effects only deliver once the sub-missile impact path
// (CProjectile::DeliverEffects, slot 0x78) is recovered; today they ride inert.
// The launcher's own m_nSpellLevel/m_casterResRef are not set on the cast path
// yet, so they propagate as the ctor defaults.
void CProjectileSPMAGMIS::Fire(CGameArea* pArea, LONG source, LONG target,
                               CPoint targetPos, LONG nHeight, SHORT nType)
{
    (void)nHeight;
    CGameObjectArray* pArray = g_pBaldurChitin->GetObjectGame()->GetObjectArray();
    BYTE rc;

    // Resolve the caster (source) position.
    CGameObject* pSource;
    do {
        rc = pArray->GetShare(source, CGameObjectArray::THREAD_ASYNCH, &pSource, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }
    CPoint ptSource = pSource->GetPos();
    pArray->ReleaseShare(source, CGameObjectArray::THREAD_ASYNCH, INFINITE);

    // Resolve the target position.
    CGameObject* pTarget;
    do {
        rc = pArray->GetShare(target, CGameObjectArray::THREAD_ASYNCH, &pTarget, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }
    CPoint ptTarget = pTarget->GetPos();
    pArray->ReleaseShare(target, CGameObjectArray::THREAD_ASYNCH, INFINITE);

    // Launch direction (source - target), the y un-squashed by 4/3.
    int dirX = ptSource.x - ptTarget.x;
    int dirY = (ptSource.y * 4) / 3 - (ptTarget.y * 4) / 3;
    int dist = static_cast<int>(
        sqrt(static_cast<double>(dirX * dirX + dirY * dirY)) + 0.5);

    int normX;
    int normY;
    if (dist == 0) {
        dist = 1;
        normX = 1;
        normY = 1;
    } else {
        normX = ((dirX << 10) * m_velocity) / dist;
        normY = ((dirY << 10) * m_velocity) / dist;
    }
    // Unit-normal length (truncated) and the running per-pair spread index.
    int normLen = static_cast<int>(sqrt(static_cast<double>(normX * normX + normY * normY)));
    int spreadIndex = 1;

    // Walk the sub-missiles two at a time, splaying each pair symmetrically.
    POSITION pos = m_subMissiles.GetHeadPosition();
    while (pos != NULL) {
        CProjectileTravelling* pA = static_cast<CProjectileTravelling*>(m_subMissiles.GetNext(pos));
        PrimeAndFireSubMissile(pA, pArea, source, target, targetPos, nType);

        CProjectileTravelling* pB = NULL;
        if (pos != NULL) {
            pB = static_cast<CProjectileTravelling*>(m_subMissiles.GetNext(pos));
            PrimeAndFireSubMissile(pB, pArea, source, target, targetPos, nType);
        }

        if (pB != NULL) {
            // Equal-and-opposite perpendicular drift, growing each pair.
            int offX = spreadIndex * normX;
            int offY = spreadIndex * normY;
            // Per-tick bleed-off of the lateral drift.  Binary 0x530C90 divides
            // by 40 (0x28); a larger divisor -> smaller decay -> the splay holds
            // longer before the missiles home in.  /10 bled the drift off ~4x too
            // fast, pulling the missiles back to centre early (a too-narrow fan).
            USHORT band = static_cast<USHORT>((spreadIndex * normLen) / 40);
            int hasOffset = (offX != 0 || offY != 0) ? 1 : 0;

            pA->m_driftX = offY;
            pA->m_driftY = -offX;
            pA->m_driftDecay = band;
            pA->m_hasDrift = hasOffset;

            pB->m_driftX = -offY;
            pB->m_driftY = offX;
            pB->m_driftDecay = band;
            pB->m_hasDrift = hasOffset;

            ++spreadIndex;
        } else {
            // Odd man out: no drift, flies straight to the target.
            pA->m_driftX = 0;
            pA->m_driftY = 0;
            pA->m_driftDecay = static_cast<USHORT>(m_velocity);
            pA->m_hasDrift = 0;
        }
    }

    m_subMissiles.RemoveAll();
    delete this;
}

// Per sub-missile launch prep shared by the pair and odd-man paths of
// CProjectileSPMAGMIS::Fire (inlined in the original at 0x530C90): clone the
// launcher's effects onto the missile, jitter its velocity, copy the caster
// fields, then fire it. The perpendicular drift is set by the caller afterwards
// (the original pokes the missile's carry fields after this returns).
void CProjectileSPMAGMIS::PrimeAndFireSubMissile(CProjectileTravelling* pMissile,
    CGameArea* pArea, LONG source, LONG target, CPoint targetPos, SHORT nType)
{
    for (POSITION ep = m_effectList.GetHeadPosition(); ep != NULL; ) {
        CGameEffect* pEffect = m_effectList.GetNext(ep);
        CGameEffect* pClone = pEffect->Copy();
        pClone->m_projectileType = pMissile->m_projectileType;
        pMissile->m_effectList.AddTail(pClone);
    }

    pMissile->m_velocity = static_cast<SHORT>(rand() % 20 + pMissile->m_velocity - 10);
    pMissile->m_nSpellLevel = m_nSpellLevel;
    pMissile->m_casterResRef = m_casterResRef;

    pMissile->Fire(pArea, source, target, targetPos, 0, nType);
}

// 0x52B190 (vtable slot 19 -- Render)
//
// Tile-based draw of a travelling projectile: visibility/passability gates,
// field-driven blit flags (mirror/tint/shadow), the directional-sequence
// pick, then the CInfinity FX pipeline. Facings past CGameSprite::DIR_N
// mirror the blit horizontally; the DIR_W..DIR_E window mirrors vertically
// and flips the draw rect. m_paletteSwap swaps the cell palette from the
// travel-palette bitmap around the main cell draw (sparkle streams).
void CProjectileTravelling::Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface)
{
    (void)pArea;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CProjectile.cpp
    // __LINE__: 4190
    UTIL_ASSERT(pVidMode != NULL);

    COLORREF tintColor = RGB(0xFF, 0xFF, 0xFF);

    DWORD flags = GetRenderFlags();
    if (m_mirror != 0) {
        flags |= 0x200;
    }

    // Tile-visibility gate (32x32 visibility tiles).
    LONG tileIndex = (m_pos.y / 32) * m_pArea->m_visibility.m_nWidth + (m_pos.x / 32);
    if (!m_pArea->m_visibility.IsTileVisible(tileIndex)) {
        return;
    }

    // Passability gate via the projectile terrain table (16x12 search cells).
    CPoint searchPos(m_pos.x / 16, m_pos.y / 12);
    if (m_pArea->m_search.GetMobileCost(searchPos, m_terrainTable, 3, TRUE) == CPathSearch::COST_IMPASSABLE) {
        return;
    }
    if (m_visible == 0) {
        return;
    }

    if (m_tinted != 0) {
        flags |= 0x10000;
    }
    if (m_hasShadowCell != 0) {
        flags |= 0x4;
    }

    // Directional projectiles (m_dirCount > 1, e.g. arrows): pick the animation
    // sequence matching the current facing before the cell is measured.
    if (m_dirCount > 1) {
        UpdateDirectionSequence(NULL);
    }

    // Clip to the area viewport. The original reads these as raw CGameArea
    // offsets (+0x514..0x560), but they live in the embedded CInfinity
    // (m_cInfinity at +0x4CC): the scroll origin (nCurrentX/Y) plus the screen
    // rect (rViewPort). Accessed by name so the layout drift does not misread
    // them (a raw +0x514 read gave an inverted viewport -> everything clipped).
    CInfinity* pInfinity = m_pArea->GetInfinity();
    CRect rViewport;
    rViewport.left = pInfinity->nCurrentX;
    rViewport.top = pInfinity->nCurrentY;
    rViewport.right = (pInfinity->rViewPort.right - pInfinity->rViewPort.left)
                      + pInfinity->nCurrentX;
    rViewport.bottom = (pInfinity->rViewPort.bottom - pInfinity->rViewPort.top)
                       + pInfinity->nCurrentY;

    CRect rFX;
    CPoint ptRef;
    GetCellBounds(rFX, ptRef, NULL);

    CPoint newPos;
    newPos.x = m_pos.x;
    LONG zOffset;
    if (m_mirror == 0 && m_hasShadowCell == 0) {
        newPos.y = m_pos.y - m_posZ;
        zOffset = 0;
    } else {
        newPos.y = m_pos.y;
        zOffset = m_posZ;
    }
    if (m_useHeightOffset != 0) {
        newPos.y += m_pArea->GetHeightOffset(m_pos, m_listType);
    }

    CRect rGCBounds(0, 0, 0, 0);
    if (CGameSprite::DIR_N < m_direction) {
        rGCBounds.left = (ptRef.x - rFX.right) + newPos.x + rFX.left;
    } else {
        rGCBounds.left = newPos.x - ptRef.x;
    }
    if (CGameSprite::DIR_W < m_direction && m_direction < CGameSprite::DIR_E) {
        if (m_mirror == 0 && m_hasShadowCell == 0) {
            rGCBounds.top = (rFX.top - rFX.bottom) + ptRef.y + newPos.y;
        } else {
            ptRef.y = (rFX.bottom - rFX.top) - ptRef.y;
            rGCBounds.top = (newPos.y - m_posZ) - ptRef.y;
        }
        zOffset = -zOffset;
    } else {
        rGCBounds.top = newPos.y - ptRef.y;
    }
    rGCBounds.right = (rFX.right - rFX.left) + rGCBounds.left;
    rGCBounds.bottom = (rFX.bottom - rFX.top) + rGCBounds.top;

    if (!IntersectRect(&rViewport, &rGCBounds, &rViewport)) {
        return;
    }

    if (m_tinted != 0 || m_mirror != 0) {
        tintColor = m_pArea->GetTintColor(newPos, m_listType);
    }

    if (CGameSprite::DIR_N < m_direction) {
        flags |= CInfinity::MIRROR_FX;
    }
    if (CGameSprite::DIR_W < m_direction && m_direction < CGameSprite::DIR_E) {
        flags |= CInfinity::MIRROR_FX_UPDOWN;
    }
    flags |= CInfinity::FXPREP_COPYFROMBACK;

    pInfinity->FXPrep(rFX, flags, nSurface, newPos, ptRef);
    if (pInfinity->FXLock(rFX, flags)) {
        if (m_tinted != 0) {
            m_pVidCell->SetTintColor(tintColor);
        }
        if (m_hasShadowCell != 0) {
            pInfinity->FXRender(m_pShadowCell, ptRef.x, ptRef.y, flags, 0);
        }
        if (m_paletteSwap == 0) {
            pInfinity->FXRender(m_pVidCell, ptRef.x, ptRef.y - zOffset, flags, 0x80);
        } else {
            m_pTravelPaletteRes->Demand();
            int nColorCount = m_pTravelPaletteRes->GetColorCount();
            RGBQUAD* pColorTable = m_pTravelPaletteRes->GetColorTable();
            m_pVidCell->SetPalette(pColorTable, nColorCount, CVidPalette::TYPE_RESOURCE);
            pInfinity->FXRender(m_pVidCell, ptRef.x, ptRef.y - zOffset, flags, 0x80);
            m_pTravelPaletteRes->Release();
        }
        pInfinity->FXRenderClippingPolys(newPos.x, newPos.y, 0, ptRef, rGCBounds, FALSE, flags);
        pInfinity->FXUnlock(flags, NULL, CPoint(0, 0));
        pInfinity->FXBltFrom(nSurface, rFX, newPos.x, newPos.y, ptRef.x, ptRef.y, flags);
    }
}

// -----------------------------------------------------------------------------

// 0x578110
//
// The shared CProjectileTravelling state (heap cell from the resref, range
// palette, velocity 0x14, lifetime 0x7FFF, render flags 0x20000, zeroed
// motion fields) is identical in the binary and comes from the base ctor;
// only the differences are written here. field_17E = "" transcribes the
// binary's explicit CString assignment of the nil string.
IcewindCProjectileTravellingVFX::IcewindCProjectileTravellingVFX(const CResRef& resRef)
    : CProjectileTravelling(resRef)
{
    // Default projectile terrain-cost table (.data 0x8A8154): terrain types
    // 0, 10 and 13 are impassable, everything else costs 5.
    m_terrainTable[0] = CPathSearch::COST_IMPASSABLE;
    m_terrainTable[1] = 5;
    m_terrainTable[2] = 5;
    m_terrainTable[3] = 5;
    m_terrainTable[4] = 5;
    m_terrainTable[5] = 5;
    m_terrainTable[6] = 5;
    m_terrainTable[7] = 5;
    m_terrainTable[8] = 5;
    m_terrainTable[9] = 5;
    m_terrainTable[10] = CPathSearch::COST_IMPASSABLE;
    m_terrainTable[11] = 5;
    m_terrainTable[12] = 5;
    m_terrainTable[13] = CPathSearch::COST_IMPASSABLE;
    m_terrainTable[14] = 5;
    m_terrainTable[15] = 5;
    field_17E = "";
    m_useHeightOffset = 1;
    m_bHasHeight = TRUE;
    m_bMirrorNorth = 1;
    m_bMirrorEast = 1;
}

// 0x578AB0 (vtable slot 3)
// The family's flight tick is a verbatim copy of CProjectileTravelling::
// AIUpdate (0x52B900): cell frame advance, 16x12-cell then radius
// (velocity+1, y weighted 16/9) arrival tests into the OnArrival virtual,
// lifetime countdown into RemoveSelf, AimAtPoint homing (point target when
// m_targetId is INVALID_INDEX), trailing sub-projectile, sound follow.
// Delegating shares that recovered body and its documented partial stubs
// (pause-gate, live-target height interpolation, trailing factory).
void IcewindCProjectileTravellingVFX::AIUpdate()
{
    CProjectileTravelling::AIUpdate();
}

// 0x5791D0 (vtable slot 27 -- the IcewindCProjectileBAM family's own launch)
//
// NOT the base CProjectileTravelling::Fire (that is a different vtable target):
// this is the VFX family's launch. It diverges from the base in the ways that
// drive the on-screen result:
//   * the projectile enters the area at the source's CAST height
//     (DetermineHeight), so the BAM renders at the caster's body -- the base
//     adds at ground level (posZ 0), which drops the visual to the feet and, in
//     isometric projection, shifts it down/forward of the caster's hands;
//   * the launch origin snaps back to the source centre when the target lands
//     within ~3 search cells of it (a near / self cast);
//   * a non-creature (point) target forces m_flightDistSq to the non-zero
//     in-flight marker 1 and the launch delta-Z to 0.
// The per-tick integrator (AIUpdate / AimAtPoint) is the base's, shared by the
// family's delegating overrides.
//
// The trailing-object factory FUN_00554d20 remains a documented stub — all
// recovered projectile leaves carry field_17E="" so the branch is dead.
void IcewindCProjectileTravellingVFX::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    (void)nHeight;
    (void)nType;

    m_targetId = target;
    m_sourceId = source;
    m_pArea = pArea;

    // Target point: the live target position when homing, else the passed point.
    // bTargetCreature gates the delta-Z height refinement via CalculateFxRect
    // (CGameAnimationType vtable slot 1); the binary seeds default values for
    // a point / non-creature target and overrides them from the live animation
    // when the target is a sprite — see 0x5792BD–0x57931B.
    CPoint ptTarget;
    BOOL bTargetCreature = FALSE;
    CRect rHeight(0x20, 0x32, 0x40, 0x40);
    CPoint ptRef(0, 0);
    if (m_targetId == CGameObjectArray::INVALID_INDEX) {
        ptTarget = targetPos;
    } else {
        CGameObject* pTarget;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
                CGameObjectArray::THREAD_ASYNCH, &pTarget, INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }
        bTargetCreature = (pTarget->GetObjectType() == CGameObject::TYPE_SPRITE);
        ptTarget = pTarget->GetPos();
        if (bTargetCreature) {
            // The original upgrades to an exclusive lock (GetDeny) to read the
            // live animation height rect, then releases back to the shared lock
            // for the remainder of the target block (0x57929E–0x57931B).
            CGameSprite* pTargetSprite = static_cast<CGameSprite*>(pTarget);
            BYTE rcDeny;
            do {
                rcDeny = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
                    CGameObjectArray::THREAD_ASYNCH, &pTarget, INFINITE);
            } while (rcDeny == CGameObjectArray::SHARED || rcDeny == CGameObjectArray::DENIED);
            if (rcDeny == CGameObjectArray::SUCCESS) {
                pTargetSprite->GetAnimation()->CalculateFxRect(rHeight, ptRef, pTargetSprite->m_posZ);
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
                    CGameObjectArray::THREAD_ASYNCH, INFINITE);
            }
            // On deny failure, keep the defaults (rHeight/ptRef already seeded).
        }
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }

    // Source: the cast height (the launch posZ) and the centre position (the
    // near-target snap fallback), plus whether the source is a creature.
    LONG nLaunchHeight = 0;
    CPoint ptSourceCenter(0, 0);
    BOOL bSourceCreature = FALSE;
    {
        CGameObject* pSource;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
                CGameObjectArray::THREAD_ASYNCH, &pSource, INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }
        nLaunchHeight = DetermineHeight(static_cast<CGameSprite*>(pSource));
        ptSourceCenter = pSource->GetPos();
        bSourceCreature = (pSource->GetObjectType() == CGameObject::TYPE_SPRITE);
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }

    // Launch origin: the facing-adjusted source point, snapped to the source
    // centre when the target lands within ~3 search cells (a near / self cast).
    CPoint ptLaunch;
    GetStart(m_sourceId, ptLaunch, TRUE);
    if (bSourceCreature) {
        int cellDX = ptTarget.x / 16 - ptLaunch.x / 16;   // 16-wide search cells
        int cellDY = ptTarget.y / 12 - ptLaunch.y / 12;   // 12-tall search cells
        if (cellDX * cellDX + cellDY * cellDY < 3) {
            ptLaunch = ptSourceCenter;
        }
    }

    // Trailing VFX object: when field_17E carries a resource name the
    // original creates a trailing game-object (a sprite or VFX that follows
    // the projectile) through the factory at 0x554D20, stores its id in
    // m_nTargetId, and dispatches a CMessageProjectileTrailingVFX (vtable
    // 0x84D328) to attach it — see 0x5794CF–0x5796BC.
    //
    // The factory (0x554D20, ~190 bytes of asm):
    //   Constructs a CResRef from this->field_17E, looks it up in the DIMM
    //   key table (type 0x3FC), creates a CGameObject, registers it in the
    //   object array and area at the launch position, seeds its subpixel
    //   position (+0x28E/+0x292), and returns its id.  The message carries
    //   the launch parameters (position, height, type, flight distance) and
    //   is delivered via CMessageHandler::AddMessage; after dispatch the
    //   target object's fields at +0x82 (projectile type) and +0x9A |= 1
    //   (the "active" flag) are updated.
    //
    // The recovered leaves (cone/spray/Chromatic Orb/Sparkle) all carry an
    // empty field_17E, so the branch is never taken. Recover the full
    // factory when a leaf that sets field_17E is needed.
    if (m_nTargetId == CGameObjectArray::INVALID_INDEX && !field_17E.IsEmpty()) {
        // STUB: m_nTargetId = CreateTrailingObject(pArea, ptLaunch, nLaunchHeight,
        //       m_targetId, ptTarget, nHeight, nType);
        // → 0x554D20.  Currently unrecovered; the branch is dead for
        //   all recovered projectile leaves (field_17E stays "").

        CMessageProjectileTrailingVFX* pMsg = new CMessageProjectileTrailingVFX(
            m_targetId, m_flightDistSq, CResRef(field_17E),
            ptLaunch.x, ptLaunch.y, nLaunchHeight, nType,
            static_cast<SHORT>(nLaunchHeight));

        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMsg, FALSE);

        // Post-dispatch: mark the trailing object with the projectile type
        // and the "active" flag (0x579621–0x5796A8). The original also
        // pauses until the object is available (GetDeny loop).
        CGameObject* pTrailingObj;
        BYTE rcMsg;
        do {
            rcMsg = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(
                m_nTargetId, CGameObjectArray::THREAD_ASYNCH, &pTrailingObj, INFINITE);
        } while (rcMsg == CGameObjectArray::SHARED || rcMsg == CGameObjectArray::DENIED);
        if (rcMsg == CGameObjectArray::SUCCESS) {
            // The binary writes the WORD at projectile+0x70 (m_projectileType)
            // to the trailing object at +0x82, and sets bit 0 of +0x9A.
            // These are currently unrecovered fields on the trailing object.
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(
                m_nTargetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    }

    // Flight distance^2 (y weighted 16/9) -- AIUpdate's arrival metric; drives
    // the distance-based lifetime when armed.
    int dx = ptTarget.x - ptLaunch.x;
    int dy = ptTarget.y - ptLaunch.y;
    m_flightDistSq = dx * dx + (dy * dy * 16) / 9;
    if (m_distLifetime != 0) {
        m_lifetime = static_cast<SHORT>(
            static_cast<int>(sqrt(static_cast<double>(m_flightDistSq))) / m_velocity + 1);
    }

    // A point / non-creature target overwrites the metric with the in-flight
    // marker 1 and launches flat; a creature target keeps the real distance and
    // refines the launch delta-Z from its animation height rect (the bottom
    // edge minus the reference point's y, halved — 0x57974E–0x579760).
    SHORT nDeltaZ;
    if (bTargetCreature) {
        nDeltaZ = static_cast<SHORT>(nLaunchHeight - (rHeight.bottom - ptRef.y) / 2);
    } else {
        m_flightDistSq = 1;
        nDeltaZ = 0;
    }
    m_nDeltaZ = nDeltaZ;
    m_nDeltaZLast = nDeltaZ;

    // Register (assigns m_id), then add to the area AT THE CAST HEIGHT -- the
    // difference from the base launch, which adds at ground level (posZ 0).
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    AddToArea(pArea, ptLaunch, nLaunchHeight, 0);

    PlaySound(m_fireSoundRef, FALSE, FALSE);

    // Subpixel launch position (1/1024 fixed point), target point, initial facing.
    m_posAccumX = ptLaunch.x << 10;
    m_posAccumY = (ptLaunch.y << 12) / 3;
    m_targetX = ptTarget.x;
    m_targetY = ptTarget.y;
    m_facing = GetDirection(ptTarget);
}

// 0x578ED0 (vtable slot 33)
// UNIMPLEMENTED: the family's own flight aiming; delegate to the recovered
// CProjectileTravelling integrator until it is recovered.
void IcewindCProjectileTravellingVFX::AimAtPoint(int x, int y)
{
    CProjectileTravelling::AimAtPoint(x, y);
}

// 0x578480 (vtable slot 19)
//
// The family draw: unlike CProjectileTravelling::Render the blit flags come
// from the attached visual effect (copy-from-back arms the translucent 0x200
// path), the area tint is applied when tinting or copy-from-back is on, and
// the east/north facings mirror through the m_bMirror* gates.
void IcewindCProjectileTravellingVFX::Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface)
{
    (void)pArea;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectileBAM.cpp
    // __LINE__: 140
    UTIL_ASSERT(pVidMode != NULL);

    DWORD flags = m_visualEffect.m_dwFlags;
    COLORREF tintColor = RGB(0xFF, 0xFF, 0xFF);

    // Tile-visibility gate (32x32 visibility tiles).
    LONG tileIndex = (m_pos.y / 32) * m_pArea->m_visibility.m_nWidth + (m_pos.x / 32);
    if (!m_pArea->m_visibility.IsTileVisible(tileIndex)) {
        return;
    }

    // Passability gate via the seeded terrain table (16x12 search cells).
    CPoint searchPos(m_pos.x / 16, m_pos.y / 12);
    if (m_pArea->m_search.GetMobileCost(searchPos, m_terrainTable, 3, TRUE) == CPathSearch::COST_IMPASSABLE) {
        return;
    }
    if (m_visible == 0) {
        return;
    }

    if (m_hasShadowCell != 0) {
        flags |= 0x4;
    }

    if (m_dirCount > 1) {
        UpdateDirectionSequence(NULL);
    }

    // Viewport from the area's embedded CInfinity, accessed by name (see
    // CProjectileTravelling::Render).
    CInfinity* pInfinity = m_pArea->GetInfinity();
    CRect rViewport;
    rViewport.left = pInfinity->nCurrentX;
    rViewport.top = pInfinity->nCurrentY;
    rViewport.right = (pInfinity->rViewPort.right - pInfinity->rViewPort.left)
                      + pInfinity->nCurrentX;
    rViewport.bottom = (pInfinity->rViewPort.bottom - pInfinity->rViewPort.top)
                       + pInfinity->nCurrentY;

    CRect rFX;
    CPoint ptRef;
    GetCellBounds(rFX, ptRef, NULL);

    CPoint newPos;
    newPos.x = m_pos.x;
    newPos.y = m_pos.y;
    LONG zOffset;
    if (m_hasShadowCell == 0) {
        newPos.y -= m_posZ;
        zOffset = 0;
    } else {
        zOffset = m_posZ;
    }
    if (m_useHeightOffset != 0 || m_bHasHeight) {
        newPos.y += m_pArea->GetHeightOffset(m_pos, m_listType);
    }

    CRect rGCBounds(0, 0, 0, 0);
    if (CGameSprite::DIR_N < m_direction) {
        rGCBounds.left = (ptRef.x - rFX.right) + newPos.x + rFX.left;
    } else {
        rGCBounds.left = newPos.x - ptRef.x;
    }
    if (CGameSprite::DIR_W < m_direction && m_direction < CGameSprite::DIR_E) {
        rGCBounds.top = newPos.y;
        if (m_hasShadowCell != 0) {
            ptRef.y = (rFX.bottom - rFX.top) - ptRef.y;
            rGCBounds.top = newPos.y - m_posZ;
        }
        rGCBounds.top -= ptRef.y;
        zOffset = -zOffset;
    } else {
        rGCBounds.top = newPos.y - ptRef.y;
    }
    rGCBounds.bottom = (rFX.bottom - rFX.top) + rGCBounds.top;
    rGCBounds.right = (rFX.right - rFX.left) + rGCBounds.left;

    if (!IntersectRect(&rViewport, &rGCBounds, &rViewport)) {
        return;
    }

    if (m_visualEffect.m_bTintEnabled == TRUE || m_visualEffect.m_bCopyFromBack == TRUE) {
        tintColor = m_pArea->GetTintColor(newPos, m_listType);
    }

    if (CGameSprite::DIR_N < m_direction && m_bMirrorEast == TRUE) {
        flags |= CInfinity::MIRROR_FX;
    }
    if (CGameSprite::DIR_W < m_direction && m_direction < CGameSprite::DIR_E
        && m_bMirrorNorth == TRUE) {
        flags ^= CInfinity::MIRROR_FX;
    }

    pInfinity->FXPrep(rFX, CInfinity::FXPREP_COPYFROMBACK | flags, nSurface, newPos, ptRef);
    if (pInfinity->FXLock(rFX, flags)) {
        if (m_visualEffect.m_bTintEnabled == TRUE) {
            m_pVidCell->SetTintColor(tintColor);
        }
        if (m_hasShadowCell != 0) {
            pInfinity->FXRender(m_pShadowCell, ptRef.x, ptRef.y, flags, 0);
        }
        if (m_paletteSwap == 0) {
            pInfinity->FXRender(m_pVidCell,
                ptRef.x,
                ptRef.y - zOffset,
                flags,
                m_visualEffect.m_nTransValue);
        } else {
            m_pTravelPaletteRes->Demand();
            int nColorCount = m_pTravelPaletteRes->GetColorCount();
            RGBQUAD* pColorTable = m_pTravelPaletteRes->GetColorTable();
            m_pVidCell->SetPalette(pColorTable, nColorCount, CVidPalette::TYPE_RESOURCE);
            pInfinity->FXRender(m_pVidCell,
                ptRef.x,
                ptRef.y - zOffset,
                flags,
                m_visualEffect.m_nTransValue);
            m_pTravelPaletteRes->Release();
        }
        pInfinity->FXRenderClippingPolys(newPos.x, newPos.y, 0, ptRef, rGCBounds, FALSE, flags);
        pInfinity->FXUnlock(flags, NULL, CPoint(0, 0));
        pInfinity->FXBltFrom(nSurface, rFX, newPos.x, newPos.y, ptRef.x, ptRef.y, flags);
    }
}

// 0x578970
//
// Bounding box and reference point for the draw: the cell alone, or, with a
// shadow cell, the union of the flying cell (lifted by m_posZ) and the
// ground shadow.
void IcewindCProjectileTravellingVFX::GetCellBounds(CRect& rBounds, CPoint& ptRef, CVidCell* pCell)
{
    CSize size;

    if (pCell == NULL) {
        pCell = m_pVidCell;
    }

    if (m_hasShadowCell == 0) {
        pCell->GetCurrentCenterPoint(ptRef, FALSE);
        pCell->GetCurrentFrameSize(size, FALSE);
        rBounds.SetRect(0, 0, size.cx, size.cy);
    }

    if (m_hasShadowCell != 0) {
        CPoint cellCenter;
        CPoint shadowCenter;

        pCell->GetCurrentCenterPoint(cellCenter, FALSE);
        cellCenter.y += m_posZ;
        m_pShadowCell->GetCurrentCenterPoint(shadowCenter, FALSE);

        ptRef.x = cellCenter.x;
        ptRef.y = cellCenter.y;
        if (ptRef.x < shadowCenter.x) {
            ptRef.x = shadowCenter.x;
        }
        if (ptRef.y < shadowCenter.y) {
            ptRef.y = shadowCenter.y;
        }

        pCell->GetCurrentFrameSize(size, FALSE);
        rBounds.SetRect(0,
            0,
            size.cx + (ptRef.x - cellCenter.x),
            size.cy + (ptRef.y - cellCenter.y));

        m_pShadowCell->GetCurrentFrameSize(size, FALSE);
        if (rBounds.right < size.cx + (ptRef.x - shadowCenter.x)) {
            rBounds.right = size.cx + (ptRef.x - shadowCenter.x);
        }
        if (rBounds.bottom < size.cy + (ptRef.y - shadowCenter.y)) {
            rBounds.bottom = size.cy + (ptRef.y - shadowCenter.y);
        }
    }
}

// 0x579860
//
// Facing -> animation sequence. Like CProjectileTravelling::
// UpdateDirectionSequence but mirror-aware: with m_bMirrorNorth the BAM has
// no north-half sequences (5..8), so those facings fold onto the south ones
// (3..0) and Render flips the blit vertically through the MIRROR_FX toggle.
void IcewindCProjectileTravellingVFX::UpdateDirectionSequence(CVidCell* pCell)
{
    if (pCell == NULL) {
        pCell = m_pVidCell;
    }

    SHORT dirCount = m_dirCount;
    if (dirCount == 1) {
        return;
    }
    SHORT facing = m_facing;
    if (m_direction == facing) {
        return;
    }

    if (dirCount == 0x10) {
        switch (facing) {
        case 4: case 0xC:
            pCell->SequenceSet(4);
            break;
        case 5: case 0xB:
            if (m_bMirrorNorth == 0) {
                pCell->SequenceSet(5);
            } else {
                pCell->SequenceSet(3);
            }
            break;
        case 3: case 0xD:
            pCell->SequenceSet(3);
            break;
        case 6: case 0xA:
            if (m_bMirrorNorth == 0) {
                pCell->SequenceSet(6);
            } else {
                pCell->SequenceSet(2);
            }
            break;
        case 2: case 0xE:
            pCell->SequenceSet(2);
            break;
        case 7: case 9:
            if (m_bMirrorNorth == 0) {
                pCell->SequenceSet(7);
            } else {
                pCell->SequenceSet(1);
            }
            break;
        case 1: case 0xF:
            pCell->SequenceSet(1);
            break;
        case 8:
            if (m_bMirrorNorth == 0) {
                pCell->SequenceSet(8);
            } else {
                pCell->SequenceSet(0);
            }
            break;
        case 0:
            pCell->SequenceSet(0);
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectileBAM.cpp
            // __LINE__: 907
            UTIL_ASSERT(FALSE);
        }
    } else if (dirCount == 8) {
        switch ((facing / 2) * 2) {
        case 0: case 8:
            pCell->SequenceSet(0);
            break;
        case 2: case 6: case 0xA: case 0xE:
            pCell->SequenceSet(2);
            break;
        case 4: case 0xC:
            pCell->SequenceSet(4);
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectileBAM.cpp
            // __LINE__: 942
            UTIL_ASSERT(FALSE);
        }
    } else if (dirCount == 2) {
        switch (facing) {
        case 0: case 4: case 8: case 0xC:
            pCell->SequenceSet(0);
            break;
        case 1: case 2: case 3: case 5: case 6: case 7:
        case 9: case 0xA: case 0xB: case 0xD: case 0xE: case 0xF:
            pCell->SequenceSet(1);
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCProjectileBAM.cpp
            // __LINE__: 974
            UTIL_ASSERT(FALSE);
        }
    }

    if (pCell == m_pVidCell) {
        m_direction = m_facing;
    }
}

// -----------------------------------------------------------------------------

// 0x56EDD0
// Base ctor for the spell-hit / area-of-effect projectile family. Flies the
// invisible "SPMAGMIS" carrier, targets anyone, and broadcasts the projectile
// type byte across the per-slot flag fields. The dedup set m_miniB (binary ctor
// 0x4C4A90) and the re-strike map m_miniA (0x570D50) are real std containers now,
// so both default-construct implicitly (GatherTargets fills m_miniA). The six
// refcounted resource-name strings start empty, cleared the way 0x448D50 clears
// fresh storage (zero pointer/length/capacity, no share-count release). The
// embedded CAIObjectType, two CVidCells, three IcewindCVisualEffects and two
// CSounds construct implicitly in declaration order.
IcewindCProjectileSpellHit::IcewindCProjectileSpellHit(SHORT nType)
    : IcewindCProjectileTravellingVFX(CResRef("SPMAGMIS"))
{
    const BYTE typeByte = (BYTE)nType;

    // Base flight state the family overrides.
    m_callBackProjectile = -1;
    field_17E = "";
    m_nTargetId = -1;
    m_visible = 0;

    // Identity and target filter.
    m_aoeRange = nType;
    m_objectTag = 0x4E;
    field_2B4 = 0;
    m_bDetonated = 0;
    field_2BA = 0;
    field_2BC = 0;
    field_2FA = 0;
    field_2FE = 0;
    field_2FF = 0;
    m_bAffectNonCreatures = 0;
    m_bAnimateCell1 = 0;
    m_bAnimateCell2 = 0;
    m_targetType.Set(CAIObjectType::ANYONE);

    // The trailing scalar block before the visual slots (m_miniA, the re-strike
    // clock, default-constructs as an empty std::map).
    m_lifetime = 0x2D;
    m_strikePeriod = 10000;
    m_strikeCountdown = 0;
    m_strikeInterval = 10;
    m_bHasTravelCell = 0;

    // Three visual-emission slots: stamp each resource name's flag with the type
    // and clear the refcounted string to empty.
    m_visual1.m_cellResRef.m_flags = typeByte;
    m_visual1.m_cellResRef.m_pName = NULL;
    m_visual1.m_cellResRef.m_nameLen = 0;
    m_visual1.m_cellResRef.m_nameCap = 0;
    m_visual1.m_soundResRef.m_flags = typeByte;
    m_visual1.m_soundResRef.m_pName = NULL;
    m_visual1.m_soundResRef.m_nameLen = 0;
    m_visual1.m_soundResRef.m_nameCap = 0;

    m_visual2.m_cellResRef.m_flags = typeByte;
    m_visual2.m_cellResRef.m_pName = NULL;
    m_visual2.m_cellResRef.m_nameLen = 0;
    m_visual2.m_cellResRef.m_nameCap = 0;
    m_visual2.m_soundResRef.m_flags = typeByte;
    m_visual2.m_soundResRef.m_pName = NULL;
    m_visual2.m_soundResRef.m_nameLen = 0;
    m_visual2.m_soundResRef.m_nameCap = 0;
    m_visual2AnimMode = 0;
    m_visual2MaxSpawn = 0x7FFFFFFF;

    m_visual3.m_cellResRef.m_flags = typeByte;
    m_visual3.m_cellResRef.m_pName = NULL;
    m_visual3.m_cellResRef.m_nameLen = 0;
    m_visual3.m_cellResRef.m_nameCap = 0;
    m_visual3.m_soundResRef.m_flags = typeByte;
    m_visual3.m_soundResRef.m_pName = NULL;
    m_visual3.m_soundResRef.m_nameLen = 0;
    m_visual3.m_soundResRef.m_nameCap = 0;
    m_visual3CellPool = 0;
    m_visual3LastCellIndex = 0;
    m_visual3RespawnFlag = 0;
    m_visual3AnimMode = 0;
    m_visual3AnimFlag36 = 0;
    m_visual3DensityBase = 0xFA;
    m_visual3EmitPeriod = 6;
    m_visual3DensityRampDiv = 0x1E;
    m_visual3CloudFlag = 0;

    // m_miniB (std::set) default-constructs here -- the binary's stamps of its
    // two _Alval pad bytes (= type byte, don't-care) and _Multi flag (= 0) are
    // subsumed by the std::set ctor (0x4C4A90), which also sets _Myhead/_Mysize.
}

// -----------------------------------------------------------------------------

// Drop this slot's share of the reference-counted block (the buffer model
// CProjectileCone uses: a single heap allocation whose first byte is the share
// count and whose character data begins one byte in). The binary reaches this
// through the shared IE-string clear 0x448D50 (called with release); a count of 0
// (sole owner) or the 0xFF sentinel frees the block, otherwise the count drops.
void IcewindCProjectileSpellHit::ResName::Release()
{
    if (m_pName != NULL) {
        char* block = m_pName - 1;
        char count = block[0];
        if (count == 0 || count == (char)0xFF) {
            delete[] block;
        } else {
            block[0] = count - 1;
        }
        m_pName = NULL;
        m_nameLen = 0;
        m_nameCap = 0;
    }
}

// 0x537220 (shared IE-string assign; the derived spell-hit ctors reach the
// reference-counted name slots through it, the compiler inlining some calls).
// Release any block this slot still holds, then copy `name` into a freshly owned
// one. An empty name allocates nothing.
void IcewindCProjectileSpellHit::ResName::Set(const char* name)
{
    Release();

    LONG nameLen = (LONG)strlen(name);
    if (nameLen != 0) {
        LONG nameCap = nameLen | 0x1F;          // capacity rounded up (0x44BE10)
        char* block = new char[nameCap + 2];    // share byte + data + terminator
        block[0] = 0;                           // sole owner
        m_pName = block + 1;
        memcpy(m_pName, name, nameLen);
        m_pName[nameLen] = '\0';
        m_nameLen = nameLen;
        m_nameCap = nameCap;
    }
}

// 0x56F1F0 (the spell-hit base destructor; vtable slot 0 is the scalar deleting
// thunk 0x56F070). Releases the six reference-counted visual-slot names in
// reverse order (the binary calls the shared clear 0x448D50 for the three "B"
// names and inlines the three "A" names); the two CSounds, two CVidCells and the
// CAIObjectType target filter then destroy implicitly, then the base subobject.
// m_miniB (std::set) and m_miniA (std::map) now destroy implicitly too,
// reproducing the binary's container teardown (m_miniB's set 0x5370C0 first, then
// m_miniA's map nodes + the shared-nil release at DAT_008e3e38).
IcewindCProjectileSpellHit::~IcewindCProjectileSpellHit()
{
    m_visual3.m_soundResRef.Release();
    m_visual3.m_cellResRef.Release();
    m_visual2.m_soundResRef.Release();
    m_visual2.m_cellResRef.Release();
    m_visual1.m_soundResRef.Release();
    m_visual1.m_cellResRef.Release();
}

// 0x56F3F0
void IcewindCProjectileSpellHit::Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface)
{
    IcewindCProjectileTravellingVFX::Render(pArea, pVidMode, nSurface);
}

// -----------------------------------------------------------------------------

// 0x56F820 (vtable slot 27). The launch. Records the source/target/area, shares
// the caster to read the launch origin (GetStart) and the
// drop height (DetermineHeight), and pulls the lifetime (m_lifetime) from the
// trailing effect on m_effectList through DetermineLifetime. A homing shot starts
// at that origin; a non-homing one (m_bHasTravelCell == 0) starts on the target so it
// arrives on the first tick. After registering in the object array (assigns m_id)
// and the area, it plays the fire sound and seeds the subpixel flight: position
// accumulators at the origin (1/1024 fixed point, y squashed 4/3), a velocity-
// scaled step toward the target, and the initial facing. A zero flight distance
// arrives immediately.
void IcewindCProjectileSpellHit::Fire(CGameArea* pArea, LONG source, LONG target,
                                      CPoint targetPos, LONG nHeight, SHORT nType)
{
    (void)nHeight;
    (void)nType;

    if (pArea == NULL) {
        return;
    }

    m_sourceId = source;
    m_targetId = target;
    m_pArea = pArea;

    CGameObject* pSource;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH, &pSource, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    if (!m_effectList.IsEmpty()) {
        CGameEffect* pLast = m_effectList.GetTail();
        if (pLast != NULL) {
            m_lifetime = DetermineLifetime(static_cast<BYTE>(pLast->m_firstCall));
        }
    }

    CPoint ptLaunch;
    GetStart(m_sourceId, ptLaunch, TRUE);
    if (m_bHasTravelCell == 0) {
        ptLaunch = targetPos;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE)
        != CGameObjectArray::SUCCESS) {
        LONG savedSource = m_sourceId;
        delete this;
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(savedSource,
            CGameObjectArray::THREAD_ASYNCH, INFINITE);
        return;
    }

    LONG height = DetermineHeight(static_cast<CGameSprite*>(pSource));
    AddToArea(pArea, ptLaunch, height, CGameObject::LIST_FRONT);
    PlaySound(m_fireSoundRef, m_loopFireSound, FALSE);

    m_targetX = targetPos.x;
    m_pos.x = ptLaunch.x;
    m_posAccumX = ptLaunch.x << 10;
    m_posAccumY = (ptLaunch.y << 12) / 3;
    m_targetY = targetPos.y;
    m_pos.y = ptLaunch.y;

    int dy = (m_targetY << 2) / 3 - (m_pos.y << 2) / 3;
    int dx = m_targetX - m_pos.x;
    int dist = static_cast<int>(sqrt(static_cast<double>(dx * dx + dy * dy)));
    if (dist == 0) {
        OnArrival();
    } else {
        m_stepX = ((dx << 10) * m_velocity) / dist;
        m_stepY = ((dy << 10) * m_velocity) / dist;
        m_facing = GetDirection(CPoint(m_targetX, m_targetY));
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH, INFINITE);
}

// -----------------------------------------------------------------------------

// 0x56FAF0 (vtable slot 3). The spell-hit family's per-tick update. Frozen by
// Time Stop unless this projectile belongs to the time-stop caster. While
// travelling (m_bDetonated == 0) it flies toward the target point: snapping there
// instantly when m_bHasTravelCell is clear, otherwise homing in by m_velocity per tick
// (advancing m_pVidCell, re-aiming, dragging the travel sound) until it reaches
// the arrival radius, then OnArrival. Once detonating (m_bDetonated != 0) it advances
// its two vid cells, runs a strike pass every m_strikeCountdown ticks (gather the due
// victims, strike each), loops its area sound, and removes itself when the
// m_lifetime lifetime expires.
void IcewindCProjectileSpellHit::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    if (m_bDetonated == 0) {
        if (m_bHasTravelCell == 0) {
            m_pos.x = m_targetX;
            m_pos.y = m_targetY;
            OnArrival();
            return;
        }

        m_pVidCell->FrameAdvance();

        int dy = m_targetY - m_pos.y;
        int dx = m_targetX - m_pos.x;
        int radius = m_velocity + 1;
        if (radius * radius < (dy * dy * 16) / 9 + dx * dx) {
            AimAtPoint(m_targetX, m_targetY);
            // Trailing sparkle sub-emitter (m_bSparkleTrail) via the unrecovered
            // factory 0x51AE40 -- omitted like the same documented stub in
            // CProjectileTravelling::AIUpdate.
            m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
            return;
        }

        OnArrival();
        return;
    }

    if (m_bAnimateCell1 != 0) {
        m_cell1.FrameAdvance();
    }
    if (m_bAnimateCell2 != 0) {
        m_cell2.FrameAdvance();
    }

    if (--m_strikeCountdown < 1) {
        std::list<LONG> due = GatherTargets();
        Strike(due);
        m_strikeCountdown = m_strikePeriod;
    }

    // +0x558 sound-loop gate (inside the guessed m_visual3 slot): keep the area
    // sound looping while it is set and not already playing.
    if (m_visual3.m_soundResRef.m_nameLen != 0 && !m_sound2.IsSoundPlaying()) {
        m_sound2.Play(m_pos.x, m_pos.y, 0, 0);
    }

    if (--m_lifetime < 1) {
        RemoveSelf();
    }
}

// -----------------------------------------------------------------------------

// 0x56F410 (vtable slot 28). Arrival: when a call-back projectile was registered,
// share it and run its CallBack hook, then flip into the detonation state
// (m_bDetonated = 1, which AIUpdate uses to start strike passes), drop the render
// gate, and play the arrival sound.  Then spawn the detonation FX: the shared
// cell pool (IcewindCSpellHitCellPool), the on-ground visual
// (IcewindCSpellHitVisual), the impact sound from the first emission slot's
// second resref, and the looping ambience from the ranged slot's second resref.
void IcewindCProjectileSpellHit::OnArrival()
{
    if (m_callBackProjectile != CGameObjectArray::INVALID_INDEX) {
        CGameObject* pCallback;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_callBackProjectile,
                CGameObjectArray::THREAD_ASYNCH,
                &pCallback,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        static_cast<CProjectile*>(pCallback)->CallBack();
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_callBackProjectile,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    m_bDetonated = 1;
    m_visible = 0;
    PlaySound(m_arrivalSoundRef, m_loopArrivalSound, TRUE);

    // The three visual slots are the detonation's spell-hit emission descriptors
    // (their ResName/visual-effect prefix plus the trailing flag fields the visual
    // ctor copies in whole).
    const IcewindCSpellHitEmission& emission0 =
        reinterpret_cast<const IcewindCSpellHitEmission&>(m_visual1);
    const IcewindCSpellHitEmission& emission1 =
        reinterpret_cast<const IcewindCSpellHitEmission&>(m_visual2);
    IcewindCSpellHitEmissionRanged& emission2 =
        reinterpret_cast<IcewindCSpellHitEmissionRanged&>(m_visual3);

    // The ranged slot owns the shared fan-cell pool the visual and its particles share.
    if (emission2.m_resref0Len != 0) {
        emission2.m_cellPool = new IcewindCSpellHitCellPool();
    }

    // Spawn the on-ground detonation visual when any emission slot is active.
    if (emission1.m_resref0Len != 0 || emission2.m_resref0Len != 0 || emission0.m_resref0Len != 0) {
        new IcewindCSpellHitVisual(emission0, emission1, emission2, m_pArea, m_pos, m_aoeRange,
            static_cast<BYTE>(m_velocity), CGameTemporal::COLLISION_DESTROY,
            static_cast<SHORT>(m_lifetime));
    }

    // Impact one-shot from the first slot's second resref.
    if (emission0.m_resref1Len != 0) {
        m_sound1.SetResRef(CResRef(emission0.m_resref1), TRUE, TRUE);
        m_sound1.SetChannel(0xE, reinterpret_cast<DWORD>(m_pArea));
        m_sound1.Play(m_pos.x, m_pos.y, 0, FALSE);
    }

    // Looping ambience from the ranged slot's second resref.
    if (emission2.m_resref1Len != 0) {
        m_sound2.SetResRef(CResRef(emission2.m_resref1), TRUE, TRUE);
        m_sound2.SetLoopingFlag(TRUE);
        m_sound2.SetChannel(0xE, reinterpret_cast<DWORD>(m_pArea));
    }
}

// -----------------------------------------------------------------------------

// 0x78E730 (vtable slot 34). Detonation hook -- a no-op in the spell-hit base and
// every leaf (the binary folds all of them onto the shared empty 0x78E730). Present
// for vtable layout; no recovered path invokes it. Name is a guess (see header).
void IcewindCProjectileSpellHit::Explode()
{
}

// 0x5703E0 (vtable slot 35). Base lifetime getter: echo the m_lifetime default,
// ignoring the trailing effect's first-call byte. Subclasses override it to derive
// the projectile's lifetime from that effect.
LONG IcewindCProjectileSpellHit::DetermineLifetime(BYTE bFirstCall)
{
    (void)bFirstCall;
    return m_lifetime;
}

// 0x5704E0.  Seed the strike's target filter from the source's allegiance so the
// staggered bolts hit the source's enemies: a good source (EnemyAlly <=
// GOODCUTOFF) strikes ENEMY, anyone else strikes ALLY.  A local CAIObjectType
// carries just the EnemyAlly, copied into m_targetType (GatherTargets filters
// GetCloseObjects with it).
void IcewindCProjectileSpellHit::SetStrikeTargetFilter(CGameObject* pSource)
{
    if (pSource == NULL) {
        return;
    }

    CAIObjectType filter;
    if (pSource->GetAIType().GetEnemyAlly() > CAIObjectType::EA_GOODCUTOFF) {
        filter.m_nEnemyAlly = CAIObjectType::EA_ALLY;
    } else {
        filter.m_nEnemyAlly = CAIObjectType::EA_ENEMY;
    }
    m_targetType.Set(filter);
}

// 0x5703F0.  The mirror of SetStrikeTargetFilter above: byte-for-byte the same
// shape with the two EnemyAlly constants swapped, so the strike pass gathers the
// source's OWN side instead of its enemies -- a good source (EnemyAlly <=
// GOODCUTOFF) strikes ALLY, anyone else strikes ENEMY.  DecodeProjectile arms it
// on the beneficial area spells, which detonate on the party rather than on the
// creatures around it.
void IcewindCProjectileSpellHit::SetStrikeAllyFilter(CGameObject* pSource)
{
    if (pSource == NULL) {
        return;
    }

    CAIObjectType filter;
    if (pSource->GetAIType().GetEnemyAlly() > CAIObjectType::EA_GOODCUTOFF) {
        filter.m_nEnemyAlly = CAIObjectType::EA_ENEMY;
    } else {
        filter.m_nEnemyAlly = CAIObjectType::EA_ALLY;
    }
    m_targetType.Set(filter);
}

// -----------------------------------------------------------------------------

// 0x56FED0 (vtable slot 36). The gather pass: collect every m_targetType object
// within m_aoeRange of m_pos (front list from the projectile's own vert-list node,
// then the back list) with line of sight through m_terrainTable, then turn the
// scan into the list of ids due a strike this pass. Tracked victims (m_miniA)
// that left the scan radius are dropped; each scanned id is inserted on first
// sight ({id, 0}) and is due when its in-range pass count is a multiple of
// m_strikeInterval -- so on first contact -- while the pass count always advances.
// Fuses what IcewindCProjectileTargetMap splits across GatherTargets (the scan)
// and CollectDueStrikes (the interval filter).
std::list<LONG> IcewindCProjectileSpellHit::GatherTargets()
{
    CTypedPtrList<CPtrList, LONG*> targets(10);

    m_pArea->GetCloseObjects(m_posVertList, m_pos, m_targetType, m_aoeRange,
        m_terrainTable, targets, TRUE, m_bAffectNonCreatures);
    m_pArea->GetAllInRangeBack(m_pos, m_targetType, m_aoeRange,
        m_terrainTable, targets, TRUE, FALSE, m_bAffectNonCreatures);

    std::map<LONG, int>::iterator it = m_miniA.begin();
    while (it != m_miniA.end()) {
        BOOL bInRange = FALSE;
        POSITION pos = targets.GetHeadPosition();
        while (pos != NULL) {
            if (reinterpret_cast<LONG>(targets.GetNext(pos)) == it->first) {
                bInRange = TRUE;
                break;
            }
        }
        if (bInRange) {
            ++it;
        } else {
            m_miniA.erase(it++);
        }
    }

    std::list<LONG> due;
    POSITION pos = targets.GetHeadPosition();
    while (pos != NULL) {
        LONG nId = reinterpret_cast<LONG>(targets.GetNext(pos));
        if (m_miniA.find(nId) == m_miniA.end()) {
            m_miniA[nId] = 0;
        }
        if (m_miniA[nId] % m_strikeInterval == 0) {
            due.push_back(nId);
        }
        m_miniA[nId]++;
    }

    return due;
}

// -----------------------------------------------------------------------------

// 0x5701B0 (vtable slot 37). Strike every gathered victim: walk the target-id
// list and deliver a strike to each. The spell-hit family's counterpart to
// IcewindCProjectileTargetMap::Strike (which drives the wandering projectiles).
void IcewindCProjectileSpellHit::Strike(std::list<LONG>& targets)
{
    for (std::list<LONG>::iterator it = targets.begin(); it != targets.end(); ++it)
        StrikeTarget(*it);
}

// -----------------------------------------------------------------------------

// 0x5701E0 (vtable slot 38). Deliver one strike to a single gathered victim.
// Resolves the victim by object id and strikes it at most once per pass: a victim
// already in the m_miniB hit-tracker is skipped (the set stays empty for one-shot
// AOEs such as Fireball, so every gathered victim is struck once). The strike
// spawns the single-target carrier projectile (factory type m_objectTag + 1)
// loaded with a copy of every effect on this projectile, carries over the caster
// context (resref and spell level) and clears the carrier's own fire sound, then
// fires it at the victim. For sprites (object type 0x31) a dead victim is skipped
// and its position is snapshotted into m_posOld first. A victim already struck
// returns early without releasing the share, matching the binary.
void IcewindCProjectileSpellHit::StrikeTarget(LONG targetId)
{
    CGameObject* pTarget;
    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(
            targetId, CGameObjectArray::THREAD_ASYNCH, &pTarget, INFINITE)
        == CGameObjectArray::SUCCESS) {
        if (m_miniB.find(pTarget->m_id) == m_miniB.end()) {
            if ((0x01 & pTarget->GetObjectType()) != 0 || m_bAffectNonCreatures != 0) {
                if (pTarget->GetObjectType() != 0x31)
                    pTarget->GetPos();
                if (pTarget->GetObjectType() == 0x31 &&
                    IcewindMisc::IsDead(static_cast<CGameSprite*>(pTarget)))
                    return;
                if (pTarget->GetObjectType() == 0x31)
                    static_cast<CGameSprite*>(pTarget)->m_posOld = pTarget->GetPos();

                CProjectile* pChild = CProjectile::DecodeProjectile(m_objectTag + 1, NULL, 0);
                pChild->m_casterResRef = m_casterResRef;
                pChild->m_nSpellLevel = m_nSpellLevel;
                pChild->m_fireSoundRef = CResRef();
                for (POSITION pos = m_effectList.GetHeadPosition(); pos != NULL; ) {
                    CGameEffect* pEffect = m_effectList.GetNext(pos);
                    pChild->AddEffect(pEffect->Copy());
                }
                CPoint& strikePos = pTarget->GetPos();
                pChild->Fire(m_pArea, m_id, targetId, strikePos, 0x32, 0);
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                    m_callBackProjectile, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    }
}

// -----------------------------------------------------------------------------

// 0x571E80
// Fireball (SPWI304): builds through the spell-hit base (type word 0x100, whose
// zero low byte clears the broadcast flag fields), re-points the vtable to the
// leaf's own, then configures the projectile -- carrier cell "FirebaT", fire
// sound "TRA_06", the three visual slots loaded with the explosion/range BAMs
// (copy-from-back enabled), doubled launch velocity, 16 facings and m_aoeRange 200.
// The carrier-name emptiness test mirrors the base ctor's branch (an empty name
// would hide the projectile and skip the cell swap).
CProjectileFireball::CProjectileFireball()
    : IcewindCProjectileSpellHit(0x100)
{
    const char* cellName = "FirebaT";
    if (cellName[0] != '\0') {
        m_visible = 1;
        delete m_pVidCell;
        m_pVidCell = new CVidCell(CResRef(cellName), FALSE);
        m_bHasTravelCell = 1;
    } else {
        m_visible = 0;
        m_bHasTravelCell = 0;
    }

    m_fireSoundRef = CResRef("TRA_06");
    m_visualEffect.SetCopyFromBack(1);

    m_visual1.m_cellResRef.Set("FirebaX");
    m_visual1.m_soundResRef.Set("RNG_M03");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual2.m_cellResRef.Set("FirebaR");
    m_visual2.m_fx.SetCopyFromBack(1);
    m_visual2AnimMode = 1;
    m_visual2MaxSpawn = 0x14;

    m_visual3.m_cellResRef.Set("FirebaA");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_velocity = static_cast<SHORT>(m_velocity << 1);
    m_strikeCountdown = 0;
    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_visual3AnimFlag36 = 1;
    m_strikePeriod = 10000;
    m_strikeInterval = 10;
    m_lifetime = 0x2D;
    m_dirCount = 0x10;
    m_aoeRange = 200;
}

// 0x5768A0 (vtable slot 0; the scalar deleting thunk wraps this). Fireball adds
// no data of its own, so the destructor just chains to the spell-hit base.
CProjectileFireball::~CProjectileFireball()
{
}

// -----------------------------------------------------------------------------

// 0x574B80
// Stinking Cloud (SPWI213, factory type 95/0x5F). A trivial IcewindCProjectileSpellHit
// leaf like Fireball: it re-points the vtable (0x8501B0) and configures the inherited
// spell-hit state, adding no data of its own. It builds no travel cell (the cloud is
// invisible in flight) and plays no fire sound; it loads the detonation/range visuals
// ("SCloudX"/"RNG_M01" + "SCloudR" + "SCloudA") into the three emission slots with
// copy-from-back, with the persistent gas area ("ARE_M02") in the third slot's second
// resref. Lifetime (m_lifetime) 1000 (vs Fireball's 0x2D -- the cloud lingers),
// m_dirCount 1, m_aoeRange 100.
CProjectileStinkingCloud::CProjectileStinkingCloud()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("SCloudX");
    m_visual1.m_soundResRef.Set("RNG_M01");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual2.m_cellResRef.Set("SCloudR");
    m_visual2.m_fx.SetCopyFromBack(1);
    m_visual2AnimMode = 1;
    m_visual2MaxSpawn = 0xD;

    m_visual3.m_cellResRef.Set("SCloudA");
    m_visual3.m_soundResRef.Set("ARE_M02");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10;
    m_strikeInterval = 10;
    m_visual3DensityBase = 1000;
    m_lifetime = 1000;
    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_strikeCountdown = 0;
    m_dirCount = 1;
    m_aoeRange = 100;
}

// Slot-0 destructor: an empty body that chains to the spell-hit base, ICF-folded onto
// CProjectileFireball::~CProjectileFireball at 0x5768A0 (identical code). It carries no
// own address marker so it does not collide with Fireball's in the address map.
CProjectileStinkingCloud::~CProjectileStinkingCloud()
{
}

// -----------------------------------------------------------------------------

// 0x574EB0
// Web (SPWI215, factory type 64/0x40). A trivial IcewindCProjectileSpellHit leaf like
// Fireball/Stinking Cloud: it re-points the vtable (0x8502E8) and configures the
// inherited spell-hit state, adding no data of its own. Invisible in flight, no fire
// sound; it loads the web burst ("WebX"/"EFF_M19") into the first emission slot and the
// persistent web area ("WebA"/"ARE_M03") into the third, both copy-from-back, leaving
// the middle slot empty. Lifetime (m_lifetime) 0x5DC, m_aoeRange 0x96.
CProjectileWeb::CProjectileWeb()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("WebX");
    m_visual1.m_soundResRef.Set("EFF_M19");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual3.m_cellResRef.Set("WebA");
    m_visual3.m_soundResRef.Set("ARE_M03");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10;
    m_strikeInterval = 10;
    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_visual3DensityBase = 0x1C2;
    m_strikeCountdown = 0;
    m_lifetime = 0x5DC;
    m_aoeRange = 0x96;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileWeb::~CProjectileWeb()
{
}

// -----------------------------------------------------------------------------

// 0x573460
// Ice Storm (SPWI404, factory type 98/0x62). A trivial IcewindCProjectileSpellHit leaf:
// it re-points the vtable (0x84FAFC) and configures the inherited spell-hit state. It
// loads the storm burst ("IStormX", copy-from-back) into the first emission slot and
// the persistent ice area ("IStormA"/"ARE_M04") into the third, leaving the middle slot
// and the first slot's second name empty. Re-strike clock m_strikeInterval 10000, lifetime
// (m_lifetime) 100, m_dirCount 1, m_aoeRange 200.
CProjectileIceStorm::CProjectileIceStorm()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("IStormX");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual3.m_cellResRef.Set("IStormA");
    m_visual3.m_soundResRef.Set("ARE_M04");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_strikePeriod = 10;
    m_strikeCountdown = 0;
    m_strikeInterval = 10000;
    m_lifetime = 100;
    m_dirCount = 1;
    m_aoeRange = 200;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileIceStorm::~CProjectileIceStorm()
{
}

// -----------------------------------------------------------------------------

// 0x571CE0
// Entangle (SPPR105, factory type 235/0xEB). A trivial IcewindCProjectileSpellHit leaf:
// it re-points the vtable (0x84F4E4) and configures the inherited spell-hit state. It
// loads the entangle burst ("EntangX", copy-from-back) into the first emission slot and
// the persistent entangling area ("EntangA"/"ARE_P01") into the third, leaving the
// middle slot and the first slot's second name empty. Lifetime (m_lifetime) 1000,
// m_aoeRange 200.
CProjectileEntangle::CProjectileEntangle()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("EntangX");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual3.m_cellResRef.Set("EntangA");
    m_visual3.m_soundResRef.Set("ARE_P01");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10;
    m_strikeInterval = 10;
    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_visual3DensityBase = 0x1C2;
    m_strikeCountdown = 0;
    m_lifetime = 1000;
    m_aoeRange = 200;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileEntangle::~CProjectileEntangle()
{
}

// -----------------------------------------------------------------------------

// 0x572290
// Fire Storm (SPPR705, factory type 92/0x5C; the projectile is shared by SPWI081 and
// SPWI399). A trivial IcewindCProjectileSpellHit leaf: it re-points the vtable
// (0x84F6B8) and configures the inherited spell-hit state. It loads the firestorm burst
// ("FStormX"/"EFF_P45") into the first emission slot and the persistent fire area
// ("FStormA"/"ARE_P03") into the third, both copy-from-back, leaving the middle slot
// empty. Lifetime (m_lifetime) 0x69, m_dirCount 1, m_aoeRange 200.
CProjectileFireStorm::CProjectileFireStorm()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("FStormX");
    m_visual1.m_soundResRef.Set("EFF_P45");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual3.m_cellResRef.Set("FStormA");
    m_visual3.m_soundResRef.Set("ARE_P03");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10;
    m_strikeInterval = 10;
    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_strikeCountdown = 0;
    m_lifetime = 0x69;
    m_dirCount = 1;
    m_aoeRange = 200;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileFireStorm::~CProjectileFireStorm()
{
}

// -----------------------------------------------------------------------------

// 0x571170
// Acid Storm (SPWI708, factory type 211/0xD3). A trivial IcewindCProjectileSpellHit
// leaf: it re-points the vtable (0x84F274) and configures the inherited spell-hit state.
// It loads the storm burst ("AStormX", copy-from-back) into the first emission slot and
// the persistent acid area ("AStormA"/"ARE_M04") into the third, leaving the middle slot
// and the first slot's second name empty. Re-strike clock m_strikeInterval 10000, lifetime
// (m_lifetime) 0x2D, m_dirCount 1, m_aoeRange 200. It sets m_visual3AnimMode and m_visual3AnimFlag36 (not
// m_visual3RespawnFlag).
CProjectileAcidStorm::CProjectileAcidStorm()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("AStormX");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual3.m_cellResRef.Set("AStormA");
    m_visual3.m_soundResRef.Set("ARE_M04");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_visual3AnimMode = 1;
    m_visual3AnimFlag36 = 1;
    m_strikePeriod = 10;
    m_strikeCountdown = 0;
    m_strikeInterval = 10000;
    m_lifetime = 0x2D;
    m_dirCount = 1;
    m_aoeRange = 200;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileAcidStorm::~CProjectileAcidStorm()
{
}

// -----------------------------------------------------------------------------

// 0x5747A0
// Spike Stones (SPPR512, factory type 213/0xD5). A trivial IcewindCProjectileSpellHit
// leaf: it re-points the vtable (0x84FFDC) and configures the inherited spell-hit state.
// It loads the spike burst ("SStoneA"/"EFF_P48") into the first emission slot and the
// persistent spike area (the same "SStoneA" cell + "ARE_P04") into the third. Uniquely
// among the family it enables no copy-from-back on either slot. Lifetime (m_lifetime)
// 0x4B0, m_aoeRange 0x96.
CProjectileSpikeStones::CProjectileSpikeStones()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("SStoneA");
    m_visual1.m_soundResRef.Set("EFF_P48");

    m_visual3.m_cellResRef.Set("SStoneA");
    m_visual3.m_soundResRef.Set("ARE_P04");

    m_strikePeriod = 10;
    m_strikeInterval = 10;
    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_visual3DensityBase = 500;
    m_strikeCountdown = 0;
    m_lifetime = 0x4B0;
    m_aoeRange = 0x96;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileSpikeStones::~CProjectileSpikeStones()
{
}

// -----------------------------------------------------------------------------

// 0x573E90
// Power Word, Kill (SPWI903, factory type 278/0x116). A trivial IcewindCProjectileSpellHit
// leaf: it re-points the vtable (0x84FD6C) and configures the inherited spell-hit state.
// A single-burst spell-hit -- it loads only the first emission slot ("PWKillX"/"EFF_M39",
// copy-from-back) and no area slot. m_strikePeriod 10000, lifetime (m_lifetime) 0x2D, m_aoeRange 0x96.
CProjectilePowerWordKill::CProjectilePowerWordKill()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("PWKillX");
    m_visual1.m_soundResRef.Set("EFF_M39");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10000;
    m_strikeCountdown = 0;
    m_strikeInterval = 10;
    m_lifetime = 0x2D;
    m_aoeRange = 0x96;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectilePowerWordKill::~CProjectilePowerWordKill()
{
}

// -----------------------------------------------------------------------------

// 0x5778A0
// Symbol of Death (SPPR726, factory type 365/0x16D). The minimal IcewindCProjectileSpellHit
// leaf: it re-points the vtable (0x850B70), loads only the first emission slot
// ("SoPainX"/"EFF_P49", copy-from-back) and sets m_aoeRange 300 -- nothing else, so every other
// field keeps the base-ctor default.
CProjectileSymbolOfDeath::CProjectileSymbolOfDeath()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("SoPainX");
    m_visual1.m_soundResRef.Set("EFF_P49");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_aoeRange = 300;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileSymbolOfDeath::~CProjectileSymbolOfDeath()
{
}

// 0x574A70.  The Call Lightning / Static Charge strike bolt.  Unlike the
// DecodeSpellHitProjectile leaves, the base ctor's SPMAGMIS default cell is
// kept (the engine swaps the visual per strike); the ctor only sets the
// detonation radius -- every other field keeps the base-ctor default.
CProjectileCallLightningStrike::CProjectileCallLightningStrike()
    : IcewindCProjectileSpellHit(0x100)
{
    m_aoeRange = 0x113;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileCallLightningStrike::~CProjectileCallLightningStrike()
{
}

// 0x574AA0 (vtable slot 37 override). The Call Lightning bolt strikes a SINGLE
// victim, not the whole gather: where the base IcewindCProjectileSpellHit::Strike
// loops every id GatherTargets collected within m_aoeRange, this leaf delivers one
// stroke to the lead gathered target. Gated on the caster's area being outdoors
// (m_areaType bit 0) -- indoors the bolt fizzles to feedback only. With nothing in
// reach the caster plays the miss sprite-effect plus the same feedback.
void CProjectileCallLightningStrike::Strike(std::list<LONG>& targets)
{
    CGameObject* pSource;
    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(
            m_sourceId, CGameObjectArray::THREAD_ASYNCH, &pSource, INFINITE)
        == CGameObjectArray::SUCCESS) {
        CGameSprite* pSprite = NULL;
        if (pSource->GetObjectType() == 0x31)
            pSprite = static_cast<CGameSprite*>(pSource);

        if ((pSource->m_pArea->m_header.m_areaType & 1) != 0) {
            if (!targets.empty()) {
                StrikeTarget(targets.front());
            } else if (pSprite != NULL) {
                pSprite->StartSpriteEffect(2, 0, 0x1E, 0);
                pSprite->FeedBack(0x5A, 0, 0, 0, -1, 0, 0);
            }
        } else {
            pSprite->FeedBack(0x5A, 0, 0, 0, -1, 0, 0);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
            m_sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x571310
// Cloudkill (SPWI018, factory type 187/0xBB). A full three-slot IcewindCProjectileSpellHit
// cloud leaf in the Stinking Cloud mould (vtable 0x84F310): the burst slot
// ("CloudKX"/"RNG_M01"), the ring slot ("CloudKR") and the persistent gas area
// ("CloudKA"/"ARE_M02"), all copy-from-back. m_lifetime 1000, m_aoeRange 0x96.
CProjectileCloudkill::CProjectileCloudkill()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("CloudKX");
    m_visual1.m_soundResRef.Set("RNG_M01");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual2.m_cellResRef.Set("CloudKR");
    m_visual2.m_fx.SetCopyFromBack(1);
    m_visual2AnimMode = 1;
    m_visual2MaxSpawn = 0xD;

    m_visual3.m_cellResRef.Set("CloudKA");
    m_visual3.m_soundResRef.Set("ARE_M02");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10;
    m_strikeInterval = 10;
    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_visual3DensityBase = 500;
    m_strikeCountdown = 0;
    m_lifetime = 1000;
    m_dirCount = 1;
    m_aoeRange = 0x96;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileCloudkill::~CProjectileCloudkill()
{
}

// -----------------------------------------------------------------------------

// 0x5714E0
// Acid Fog (SPWI019, factory type 212/0xD4). A sibling of Cloudkill (vtable 0x84F3AC):
// the burst slot ("DFogX"/"RNG_M01"), the ring slot ("DFogR") and the persistent
// acid-gas area ("DFogA"/"ARE_M02"), all copy-from-back. m_lifetime 0x44C, m_aoeRange
// 0x96.
CProjectileAcidFog::CProjectileAcidFog()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("DFogX");
    m_visual1.m_soundResRef.Set("RNG_M01");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual2.m_cellResRef.Set("DFogR");
    m_visual2.m_fx.SetCopyFromBack(1);
    m_visual2AnimMode = 1;
    m_visual2MaxSpawn = 0x14;

    m_visual3.m_cellResRef.Set("DFogA");
    m_visual3.m_soundResRef.Set("ARE_M02");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10;
    m_strikeInterval = 10;
    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_strikeCountdown = 0;
    m_lifetime = 0x44C;
    m_dirCount = 1;
    m_aoeRange = 0x96;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileAcidFog::~CProjectileAcidFog()
{
}

// -----------------------------------------------------------------------------

// 0x572670
// Grease (SPWI101, factory type 101/0x65). A two-slot IcewindCProjectileSpellHit leaf
// (vtable 0x84F7F0, like Web): the burst slot ("GreaseX"/"EFF_M31b") and the persistent
// grease area ("GreaseA"/"ARE_M01"), both copy-from-back, leaving the middle slot empty.
// m_visual3DensityBase 1000, m_lifetime 1000, m_aoeRange 100.
CProjectileGrease::CProjectileGrease()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("GreaseX");
    m_visual1.m_soundResRef.Set("EFF_M31b");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual3.m_cellResRef.Set("GreaseA");
    m_visual3.m_soundResRef.Set("ARE_M01");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10;
    m_strikeInterval = 10;
    m_visual3DensityBase = 1000;
    m_lifetime = 1000;
    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_strikeCountdown = 0;
    m_dirCount = 1;
    m_aoeRange = 100;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileGrease::~CProjectileGrease()
{
}

// -----------------------------------------------------------------------------

// 0x571930
// Circle of Death (SPWI606, factory type 267/0x10B). A single-burst IcewindCProjectileSpellHit
// leaf (vtable 0x84F448, like Power Word Kill): the first emission slot only
// ("DSpellX"/"EFF_M42", copy-from-back) and no area slot. m_strikePeriod 10000,
// m_lifetime 0x2D, m_aoeRange 300.
CProjectileCircleOfDeath::CProjectileCircleOfDeath()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("DSpellX");
    m_visual1.m_soundResRef.Set("EFF_M42");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10000;
    m_strikeCountdown = 0;
    m_strikeInterval = 10;
    m_lifetime = 0x2D;
    m_aoeRange = 300;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileCircleOfDeath::~CProjectileCircleOfDeath()
{
}

// -----------------------------------------------------------------------------

// 0x573AF0
// Insect Plague (SPPR510, factory type 216/0xD8). A full three-slot IcewindCProjectileSpellHit
// cloud leaf (vtable 0x84FC34): the burst slot ("IPlaguX"/"RNG_P01"), the ring slot
// ("IPlaguR") and the persistent swarm area ("IPlaguA"/"ARE_P02"), all copy-from-back.
// m_lifetime 0x5DC, m_aoeRange 0xFA.
CProjectileInsectPlague::CProjectileInsectPlague()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("IPlaguX");
    m_visual1.m_soundResRef.Set("RNG_P01");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual2.m_cellResRef.Set("IPlaguR");
    m_visual2.m_fx.SetCopyFromBack(1);
    m_visual2AnimMode = 1;
    m_visual2MaxSpawn = 0xD;

    m_visual3.m_cellResRef.Set("IPlaguA");
    m_visual3.m_soundResRef.Set("ARE_P02");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10;
    m_strikeInterval = 10;
    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_strikeCountdown = 0;
    m_lifetime = 0x5DC;
    m_dirCount = 1;
    m_aoeRange = 0xFA;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileInsectPlague::~CProjectileInsectPlague()
{
}

// -----------------------------------------------------------------------------

// 0x573600
// Fiery Cloud (SPWI802, factory type 214/0xD6). A full three-slot IcewindCProjectileSpellHit
// cloud leaf (vtable 0x84FB98): burst ("ICloudX"/"EFF_M40"), ring ("ICloudR") and the
// persistent fire-cloud area ("ICloudA"/"ARE_M05"), all copy-from-back. Unique among the
// recovered leaves in setting m_visual3CloudFlag = 1 -- this is the effect that drives the
// IcewindCSpellHitParticle cloud-flip (ICloudA/ICloudB) path. m_lifetime 0x41A, m_aoeRange
// 100.
CProjectileFieryCloud::CProjectileFieryCloud()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("ICloudX");
    m_visual1.m_soundResRef.Set("EFF_M40");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual2.m_cellResRef.Set("ICloudR");
    m_visual2.m_fx.SetCopyFromBack(1);
    m_visual2AnimMode = 1;
    m_visual2MaxSpawn = 0xD;

    m_visual3.m_cellResRef.Set("ICloudA");
    m_visual3.m_soundResRef.Set("ARE_M05");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_visual3DensityBase = 0x5DC;
    m_visual3CloudFlag = 1;
    m_lifetime = 0x41A;
    m_dirCount = 1;
    m_aoeRange = 100;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileFieryCloud::~CProjectileFieryCloud()
{
}

// -----------------------------------------------------------------------------

// 0x574140
// Produce Fire (SPPR411, factory type 215/0xD7). A two-slot IcewindCProjectileSpellHit
// leaf (vtable 0x84FE08): the burst slot ("PFireX"/"EFF_P45") and the persistent fire
// area ("PFireA"/"ARE_P03"), both copy-from-back. m_strikeInterval 10000, m_lifetime 100,
// m_aoeRange 0x3C.
CProjectileProduceFire::CProjectileProduceFire()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("PFireX");
    m_visual1.m_soundResRef.Set("EFF_P45");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual3.m_cellResRef.Set("PFireA");
    m_visual3.m_soundResRef.Set("ARE_P03");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_visual3DensityBase = 0x2EE;
    m_strikePeriod = 10;
    m_strikeCountdown = 0;
    m_strikeInterval = 10000;
    m_lifetime = 100;
    m_dirCount = 1;
    m_aoeRange = 0x3C;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileProduceFire::~CProjectileProduceFire()
{
}

// -----------------------------------------------------------------------------

// 0x575B90
// Tremor (SPPR719, factory type 306/0x132). The most minimal leaf in the family (vtable
// 0x850558): it loads only the looping area sound "ARE_P27" into m_visual1.m_soundResRef
// and sets m_aoeRange 0x15E -- no cell, no copy-from-back, every other field default.
CProjectileTremor::CProjectileTremor()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_soundResRef.Set("ARE_P27");
    m_aoeRange = 0x15E;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileTremor::~CProjectileTremor()
{
}

// -----------------------------------------------------------------------------

// 0x577B20
// Dispel Magic (EFFDM1, factory type 246/0xF6). A single-burst IcewindCProjectileSpellHit
// leaf (vtable 0x850C0C): the first emission slot only, the abjuration glow
// ("AbjuraX"/"ARE_M20", copy-from-back). m_aoeRange 0x96.
CProjectileDispelMagic::CProjectileDispelMagic()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("AbjuraX");
    m_visual1.m_soundResRef.Set("ARE_M20");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_aoeRange = 0x96;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileDispelMagic::~CProjectileDispelMagic()
{
}

// -----------------------------------------------------------------------------

// The enchantment-glow overlays: four near-identical single-burst leaves loading the
// shared "EnchanX"/"ARE_M21" glow into the first emission slot (copy-from-back) and
// striking once (m_strikePeriod 10000, m_strikeInterval 10, m_lifetime 0x2D); they
// differ only by RTTI/vtable and m_aoeRange.

// 0x574300
// Sleep (SPWI116, factory type 42/0x2A; vtable 0x84FEA4; m_aoeRange 0x96).
CProjectileSleep::CProjectileSleep()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("EnchanX");
    m_visual1.m_soundResRef.Set("ARE_M21");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10000;
    m_strikeCountdown = 0;
    m_strikeInterval = 10;
    m_lifetime = 0x2D;
    m_aoeRange = 0x96;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileSleep::~CProjectileSleep()
{
}

// -----------------------------------------------------------------------------

// 0x572830
// Hold Animal (SPPR305, factory type 249/0xF9; vtable 0x84F88C; m_aoeRange 200).
CProjectileHoldAnimal::CProjectileHoldAnimal()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("EnchanX");
    m_visual1.m_soundResRef.Set("ARE_M21");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10000;
    m_strikeCountdown = 0;
    m_strikeInterval = 10;
    m_lifetime = 0x2D;
    m_aoeRange = 200;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileHoldAnimal::~CProjectileHoldAnimal()
{
}

// -----------------------------------------------------------------------------

// 0x572E60
// Eye of Stone (SPIN132, factory type 190/0xBE; vtable 0x84F9C4; m_aoeRange 0x96).
CProjectileEyeOfStone::CProjectileEyeOfStone()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("EnchanX");
    m_visual1.m_soundResRef.Set("ARE_M21");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10000;
    m_strikeCountdown = 0;
    m_strikeInterval = 10;
    m_lifetime = 0x2D;
    m_aoeRange = 0x96;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileEyeOfStone::~CProjectileEyeOfStone()
{
}

// -----------------------------------------------------------------------------

// 0x573160
// Halt Undead (SPWI320, factory type 357/0x165; vtable 0x84FA60; m_aoeRange 0x96).
CProjectileHaltUndead::CProjectileHaltUndead()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("EnchanX");
    m_visual1.m_soundResRef.Set("ARE_M21");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10000;
    m_strikeCountdown = 0;
    m_strikeInterval = 10;
    m_lifetime = 0x2D;
    m_aoeRange = 0x96;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileHaltUndead::~CProjectileHaltUndead()
{
}

// -----------------------------------------------------------------------------

// 0x5745E0
// Snilloc's Snowball Swarm (SPWI220, factory type 217/0xD9). A visible travelling
// IcewindCProjectileSpellHit leaf in the Fireball mould (vtable 0x84FF40): it builds the
// "SSSwarT" carrier cell (replacing the base cell when the name is non-empty), plays
// "TRA_18", loads the burst ("SSSwarX"/"RNG_M02") and ring ("SSSwarR") emission slots
// with copy-from-back, doubles the launch velocity and sets 16 facings. m_lifetime 0x2D,
// m_aoeRange 0xFA. The carrier-name emptiness test mirrors the base ctor's branch.
CProjectileSnowballSwarm::CProjectileSnowballSwarm()
    : IcewindCProjectileSpellHit(0x100)
{
    const char* cellName = "SSSwarT";
    if (cellName[0] != '\0') {
        m_visible = 1;
        delete m_pVidCell;
        m_pVidCell = new CVidCell(CResRef(cellName), FALSE);
        m_bHasTravelCell = 1;
    } else {
        m_visible = 0;
        m_bHasTravelCell = 0;
    }

    m_fireSoundRef = CResRef("TRA_18");
    m_visualEffect.SetCopyFromBack(1);

    m_visual1.m_cellResRef.Set("SSSwarX");
    m_visual1.m_soundResRef.Set("RNG_M02");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual2.m_cellResRef.Set("SSSwarR");
    m_visual2.m_fx.SetCopyFromBack(1);

    m_strikeCountdown = 0;
    m_velocity = static_cast<SHORT>(m_velocity << 1);
    m_visual2AnimMode = 1;
    m_visual2MaxSpawn = 0x1A;
    m_strikePeriod = 10000;
    m_strikeInterval = 10;
    m_lifetime = 0x2D;
    m_dirCount = 0x10;
    m_aoeRange = 0xFA;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileSnowballSwarm::~CProjectileSnowballSwarm()
{
}

// -----------------------------------------------------------------------------

// 0x572470
// Flame Strike (projectile type 67, SPIN977; vtable 0x84F754). The most minimal
// overlay in the family: it loads only the "EFF_P16" impact sound onto the burst
// slot -- no cell, no copy-from-back -- then the standard strike cadence. m_aoeRange
// 0x8C.
CProjectileFlameStrike::CProjectileFlameStrike()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_soundResRef.Set("EFF_P16");

    m_strikePeriod = 10000;
    m_strikeCountdown = 0;
    m_strikeInterval = 10;
    m_lifetime = 0x2D;
    m_aoeRange = 0x8C;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileFlameStrike::~CProjectileFlameStrike()
{
}

// -----------------------------------------------------------------------------

// 0x572B30
// Hold Monster (projectile type 263; vtable 0x84F928). The enchantment-glow overlay,
// identical to Hold Animal: the "EnchanX"/"ARE_M21" burst slot with copy-from-back and
// the standard strike cadence. m_aoeRange 200.
CProjectileHoldMonster::CProjectileHoldMonster()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("EnchanX");
    m_visual1.m_soundResRef.Set("ARE_M21");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10000;
    m_strikeCountdown = 0;
    m_strikeInterval = 10;
    m_lifetime = 0x2D;
    m_aoeRange = 200;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileHoldMonster::~CProjectileHoldMonster()
{
}

// -----------------------------------------------------------------------------

// 0x572110
// Fire Seed (projectile type 270; vtable 0x84F61C). A visible travelling leaf in the
// Fireball mould: it builds the "MagicStn" carrier cell (replacing the base cell when
// the name is non-empty), bursts the "FSeedsX"/"EFF_P45" overlay with copy-from-back and
// flies with 16 facings. m_lifetime 0x2D, m_aoeRange 0x46. The carrier-name emptiness
// test mirrors the base ctor's branch.
CProjectileFireSeed::CProjectileFireSeed()
    : IcewindCProjectileSpellHit(0x100)
{
    const char* cellName = "MagicStn";
    if (cellName[0] != '\0') {
        m_visible = 1;
        delete m_pVidCell;
        m_pVidCell = new CVidCell(CResRef(cellName), FALSE);
        m_bHasTravelCell = 1;
    } else {
        m_visible = 0;
        m_bHasTravelCell = 0;
    }

    m_visual1.m_cellResRef.Set("FSeedsX");
    m_visual1.m_soundResRef.Set("EFF_P45");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10000;
    m_strikeCountdown = 0;
    m_strikeInterval = 10;
    m_lifetime = 0x2D;
    m_dirCount = 0x10;
    m_aoeRange = 0x46;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileFireSeed::~CProjectileFireSeed()
{
}

// -----------------------------------------------------------------------------

// 0x573CC0
// Malavon's Corrosive Fog (projectile type 279; vtable 0x84FCD0). A full three-slot
// cloud reusing the Death Fog visuals, a sibling of Acid Fog (same "DFogX"/"RNG_M01"
// burst, "DFogR" ring and "DFogA"/"ARE_M02" persistent area, all copy-from-back) but
// with a wider ring (m_visual2MaxSpawn 0x1A), a longer 2000-tick lifetime and m_aoeRange
// 0xFA.
CProjectileCorrosiveFog::CProjectileCorrosiveFog()
    : IcewindCProjectileSpellHit(0x100)
{
    m_visual1.m_cellResRef.Set("DFogX");
    m_visual1.m_soundResRef.Set("RNG_M01");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual2.m_cellResRef.Set("DFogR");
    m_visual2.m_fx.SetCopyFromBack(1);
    m_visual2AnimMode = 1;
    m_visual2MaxSpawn = 0x1A;

    m_visual3.m_cellResRef.Set("DFogA");
    m_visual3.m_soundResRef.Set("ARE_M02");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_strikePeriod = 10;
    m_strikeInterval = 10;
    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_visual3DensityBase = 500;
    m_strikeCountdown = 0;
    m_lifetime = 2000;
    m_dirCount = 1;
    m_aoeRange = 0xFA;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileCorrosiveFog::~CProjectileCorrosiveFog()
{
}

// -----------------------------------------------------------------------------

// 0x5756C0
// Portal Animation Flipping Hack, Open (projectile type 294) and Close (projectile type
// 297) -- the same ctor and vtable (0x850420) serve both directions. Not a spell (no SPL
// owns it): the engine reuses the spell-hit projectile as a minimal portal-door overlay.
// It runs the base ctor with lifetime 2000 and resets the target type to NOT_SPRITE
// (filter @0x8C76C8, not ANYONE @0x8C7748), adding no visuals.
CProjectilePortalAnimFlip::CProjectilePortalAnimFlip()
    : IcewindCProjectileSpellHit(2000)
{
    m_targetType.Set(CAIObjectType::NOT_SPRITE);
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectilePortalAnimFlip::~CProjectilePortalAnimFlip()
{
}

// -----------------------------------------------------------------------------

// 0x577590
// Boulder, Big (Trap) (projectile type 383, SPWI088; vtable 0x850AD4). A visible
// travelling leaf (base lifetime 100): it builds the "BIGBOLDR" carrier cell, sets a
// single facing at velocity 7 and loops the "AM6103e" fire sound / "AM5101e" arrival
// sound without loading any burst overlay. m_visual3RespawnFlag is set with anim mode 0;
// m_aoeRange 100. The carrier-name emptiness test mirrors the base ctor's branch.
CProjectileBigBoulder::CProjectileBigBoulder()
    : IcewindCProjectileSpellHit(100)
{
    const char* cellName = "BIGBOLDR";
    if (cellName[0] != '\0') {
        m_visible = 1;
        delete m_pVidCell;
        m_pVidCell = new CVidCell(CResRef(cellName), FALSE);
        m_bHasTravelCell = 1;
    } else {
        m_visible = 0;
        m_bHasTravelCell = 0;
    }

    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 0;
    m_strikePeriod = 10;
    m_strikeCountdown = 0;
    m_strikeInterval = 10;
    m_dirCount = 1;
    m_aoeRange = 100;
    m_velocity = 7;
    m_fireSoundRef = CResRef("AM6103e");
    m_arrivalSoundRef = CResRef("AM5101e");
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileBigBoulder::~CProjectileBigBoulder()
{
}

// -----------------------------------------------------------------------------

// 0x576990
// Delayed Blast Fireball (projectile type 360, SPWI714; vtable 0x85099C). The own
// members m_bBlasted/m_scanTimer are cleared first, then the leaf installs a Fireball-
// shaped carrier: the visible "FirebaT" travel cell, the "TRA_06" loop, the "FirebaX"/
// "RNG_M03" burst, "FirebaR" ring and "FirebaA" area slots (all copy-from-back), a wider
// ring spawn, the cloud flags, doubled launch velocity, 16 facings and m_aoeRange 300.
CProjectileDBFireball::CProjectileDBFireball()
    : IcewindCProjectileSpellHit(0x100)
{
    m_bBlasted = 0;
    m_scanTimer = 0;

    const char* cellName = "FirebaT";
    if (cellName[0] != '\0') {
        m_visible = 1;
        delete m_pVidCell;
        m_pVidCell = new CVidCell(CResRef(cellName), FALSE);
        m_bHasTravelCell = 1;
    } else {
        m_visible = 0;
        m_bHasTravelCell = 0;
    }

    m_fireSoundRef = CResRef("TRA_06");
    m_visualEffect.SetCopyFromBack(1);

    m_visual1.m_cellResRef.Set("FirebaX");
    m_visual1.m_soundResRef.Set("RNG_M03");
    m_visual1.m_fx.SetCopyFromBack(1);

    m_visual2.m_cellResRef.Set("FirebaR");
    m_visual2.m_fx.SetCopyFromBack(1);
    m_visual2AnimMode = 1;
    m_visual2MaxSpawn = 0x14;

    m_visual3.m_cellResRef.Set("FirebaA");
    m_visual3.m_fx.SetCopyFromBack(1);

    m_strikeCountdown = 0;
    m_velocity = static_cast<SHORT>(m_velocity << 1);
    m_visual3RespawnFlag = 1;
    m_visual3AnimMode = 1;
    m_visual3AnimFlag36 = 1;
    m_strikePeriod = 10000;
    m_strikeInterval = 10;
    m_lifetime = 0x2D;
    m_dirCount = 0x10;
    m_aoeRange = 300;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileDBFireball::~CProjectileDBFireball()
{
}

// 0x576BA0 (vtable slot 3). While the bead is still flying (m_bDetonated == 0) or has
// already gone off (m_bBlasted == 1) the base AIUpdate runs unchanged. Once it has landed
// and is sitting on its delay, every fifth tick it scans its area for any object within
// 100 of its position; the first time something is in range it fires the base detonation
// (IcewindCProjectileSpellHit::OnArrival) and latches m_bBlasted. The carrier cell's frame
// is advanced every tick so the bead keeps animating while it waits.
void CProjectileDBFireball::AIUpdate()
{
    if (m_bDetonated == 0) {
        IcewindCProjectileSpellHit::AIUpdate();
        return;
    }
    if (m_bBlasted == 1) {
        IcewindCProjectileSpellHit::AIUpdate();
        return;
    }

    if (++m_scanTimer == 5) {
        m_scanTimer = 0;

        CTypedPtrList<CPtrList, LONG*> targets(10);
        GetArea()->GetCloseObjects(GetVertListPos(), GetPos(), CAIObjectType::ANYONE, 100,
            m_terrainTable, targets, TRUE, FALSE);
        GetArea()->GetAllInRangeBack(GetPos(), CAIObjectType::ANYONE, 100,
            m_terrainTable, targets, TRUE, FALSE, FALSE);

        if (targets.GetHeadPosition() != NULL) {
            IcewindCProjectileSpellHit::OnArrival();
            m_bBlasted = 1;
        }
    }

    m_pVidCell->FrameAdvance();
}

// 0x576CE0 (vtable slot 28). Landing does not detonate: it only latches the detonation
// flag so AIUpdate begins the proximity-delay loop. The blast itself is deferred to the
// first AIUpdate pass that finds something in range.
void CProjectileDBFireball::OnArrival()
{
    m_bDetonated = 1;
}

// -----------------------------------------------------------------------------

// 0x576CF0
// Turn Undead (projectile type 376; vtable 0x850A38). A minimal spell-hit leaf with no
// visuals: only the strike cadence (period 10000, interval 10), m_lifetime 0x2D and a wide
// m_aoeRange 300. It keeps the base AIUpdate/OnArrival, so it detonates on arrival like an
// ordinary overlay.
CProjectileTurnUndead::CProjectileTurnUndead()
    : IcewindCProjectileSpellHit(0x100)
{
    m_strikePeriod = 10000;
    m_strikeCountdown = 0;
    m_strikeInterval = 10;
    m_lifetime = 0x2D;
    m_aoeRange = 300;
}

// Slot-0 destructor: empty body chaining to the base, ICF-folded onto 0x5768A0.
CProjectileTurnUndead::~CProjectileTurnUndead()
{
}

// -----------------------------------------------------------------------------

// 0x57F640
// The base ctor runs with "WhirlwX" and the strike tracker / leaf sound
// members construct implicitly; this body sets the leaf state, points the
// tracker at this projectile (the tornado: scan every 3rd service within
// radius 70, re-strike after 33 in-range passes, 8 strikes total, never the
// caster), turns height handling back off, and loops the EFF_P105 fire
// sound.
CProjectileWhirlwind::CProjectileWhirlwind()
    : IcewindCProjectileTravellingVFX(CResRef("WhirlwX"))
{
    m_nLifetime = 1000;
    m_nLegBudget = 1000;
    field_2B6 = 0;
    m_bFinishing = 0;
    m_wanderSeed = 0;
    m_dirCount = 1;
    m_velocity = 5;
    m_targetMap.m_pOwner = this;
    m_targetMap.m_maxStrikesTotal = 8;
    m_targetMap.m_nRange = 70;
    m_targetMap.m_strikeInterval = 33;
    m_targetMap.m_servicePeriod = 3;
    m_targetMap.m_bSkipSource = TRUE;
    m_visualEffect.SetCopyFromBack(TRUE);
    m_bHasHeight = FALSE;
    m_useHeightOffset = 0;
    m_fireSoundRef = "EFF_P105";
    m_loopFireSound = TRUE;
}

// 0x57F760 (vtable slot 0)
// The members (strike tracker map, leaf sound, heap cell through the base
// chain) destruct implicitly.
CProjectileWhirlwind::~CProjectileWhirlwind()
{
}

// 0x57F8D0 (vtable slot 3)
// The wander tick. The original opens with the engine single-step gate
// (game +0x4B40/+0x4B44: skip unless this object is the stepped one) --
// omitted like the same documented stub in CProjectileTravelling::AIUpdate.
void CProjectileWhirlwind::AIUpdate()
{
    // Dissipating: wait for the aftermath sound, then remove and free.
    if (m_bFinishing == 1) {
        if (m_loopSound.IsSoundPlaying()) {
            return;
        }

        RemoveFromArea();
        if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(
                m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE)
            == CGameObjectArray::SUCCESS) {
            delete this;
        }
        return;
    }

    // Wander leg bookkeeping: out of budget picks the next wander point.
    if (m_nLegBudget < 1) {
        POINT nextPoint;
        PickWanderPoint(&nextPoint, FALSE);
        m_targetX = nextPoint.x;
        m_targetY = nextPoint.y;
        m_nLegBudget = rand() % 50 + 50;
    } else {
        m_nLegBudget--;
    }

    // Expired (or the strike tracker hit its total cap): go invisible, play
    // the aftermath sound and dissipate once it ends.
    if (m_nLifetime < 1 || m_targetMap.m_bDone == 1) {
        m_loopSound.SetResRef(CResRef("AFT_P23"), TRUE, TRUE);
        m_loopSound.SetFireForget(TRUE);
        m_loopSound.SetChannel(15, reinterpret_cast<DWORD>(m_pArea));
        m_loopSound.Play(m_pos.x, m_pos.y, 0, FALSE);
        m_bFinishing = 1;
        m_visible = 0;
        return;
    }
    m_nLifetime--;

    // Advance the subpixel integrator (y carries the 3/4 isometric scale).
    m_posAccumX += m_stepX;
    m_posAccumY += m_stepY;
    m_pos.x = m_posAccumX >> CGameSprite::EXACT_SCALE;
    m_pos.y = ((m_posAccumY * 3) / 4) >> CGameSprite::EXACT_SCALE;

    // Off the area: remove and free.
    if (m_pos.x < 0 || m_pos.y < 0
        || m_pArea->GetInfinity()->nAreaX <= m_pos.x
        || m_pArea->GetInfinity()->nAreaY <= m_pos.y) {
        RemoveFromArea();
        if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(
                m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE)
            == CGameObjectArray::SUCCESS) {
            delete this;
        }
        return;
    }

    // Wall collision: reflect the step off impassable squares (terrain cost
    // table FF, or structure-height code 8), preferring to flip the dominant
    // axis last, and re-aim a few ticks ahead after any bounce.
    SHORT nTableIndex;
    CPoint gridPt(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX, m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);
    if (m_pArea->m_search.GetLOSCost(gridPt, m_terrainTable, nTableIndex, FALSE) == CPathSearch::COST_IMPASSABLE
        || nTableIndex == 8) {
        int stepX = m_stepX;
        int stepY = m_stepY;
        int absX = stepX < 0 ? -stepX : stepX;
        int absY = stepY < 0 ? -stepY : stepY;

        if (absX < absY) {
            // Would flipping y alone clear the square?
            gridPt.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
            gridPt.y = ((((m_posAccumY - 2 * stepY) * 3) / 4) >> CGameSprite::EXACT_SCALE)
                / CPathSearch::GRID_SQUARE_SIZEY;
            if (m_pArea->m_search.GetLOSCost(gridPt, m_terrainTable, nTableIndex, FALSE) == CPathSearch::COST_IMPASSABLE
                || nTableIndex == 8) {
                // No: flip x first ...
                m_stepX = -stepX;
                m_posAccumX -= 2 * stepX;
                m_pos.x = m_posAccumX >> CGameSprite::EXACT_SCALE;
                gridPt.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
                gridPt.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;
                if (m_pArea->m_search.GetLOSCost(gridPt, m_terrainTable, nTableIndex, FALSE) != CPathSearch::COST_IMPASSABLE
                    && nTableIndex != 8) {
                    goto reaim;
                }
                // ... and y as well.
                m_stepY = -stepY;
                m_posAccumY -= 2 * stepY;
                m_pos.y = ((m_posAccumY * 3) / 4) >> CGameSprite::EXACT_SCALE;
            } else {
                // Yes: flip y alone.
                m_stepY = -stepY;
                m_posAccumY -= 2 * stepY;
                m_pos.y = ((m_posAccumY * 3) / 4) >> CGameSprite::EXACT_SCALE;
            }
        } else {
            // Would flipping x alone clear the square?
            gridPt.x = ((m_posAccumX - 2 * stepX) >> CGameSprite::EXACT_SCALE)
                / CPathSearch::GRID_SQUARE_SIZEX;
            gridPt.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;
            if (m_pArea->m_search.GetLOSCost(gridPt, m_terrainTable, nTableIndex, FALSE) == CPathSearch::COST_IMPASSABLE
                || nTableIndex == 8) {
                // No: flip y first ...
                m_stepY = -stepY;
                m_posAccumY -= 2 * stepY;
                m_pos.y = ((m_posAccumY * 3) / 4) >> CGameSprite::EXACT_SCALE;
                gridPt.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
                gridPt.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;
                if (m_pArea->m_search.GetLOSCost(gridPt, m_terrainTable, nTableIndex, FALSE) == CPathSearch::COST_IMPASSABLE) {
                    // ... and x as well.
                    m_posAccumX -= 2 * stepX;
                    m_stepX = -stepX;
                    m_pos.x = m_posAccumX >> CGameSprite::EXACT_SCALE;
                }
            } else {
                // Yes: flip x alone.
                m_posAccumX -= 2 * stepX;
                m_stepX = -stepX;
                m_pos.x = m_posAccumX >> CGameSprite::EXACT_SCALE;
            }
        }

    reaim:
        // Re-aim a few steps ahead along the reflected heading and start a
        // fresh leg.
        int aheadX = (m_posAccumX + 3 * m_stepX) >> CGameSprite::EXACT_SCALE;
        int aheadY = (((m_posAccumY + 3 * m_stepY) * 3) / 4) >> CGameSprite::EXACT_SCALE;
        m_facing = GetDirection(CPoint(aheadX, aheadY));
        m_targetX = aheadX;
        m_targetY = aheadY;
        m_nLegBudget = rand() % 50 + 50;
    }

    // Service the strike tracker, keep the looping sound on the tornado, and
    // start a new leg once the current wander point is reached (+/-32 x,
    // +/-24 y).
    m_targetMap.Service();
    m_loopSound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
    if (m_targetX < m_pos.x + 0x20 && m_pos.x - 0x20 < m_targetX
        && m_targetY < m_pos.y + 0x18 && m_pos.y - 0x18 < m_targetY) {
        POINT nextPoint;
        PickWanderPoint(&nextPoint, FALSE);
        m_targetX = nextPoint.x;
        m_targetY = nextPoint.y;
        m_nLegBudget = rand() % 50 + 50;
    }

    IcewindCProjectileTravellingVFX::AIUpdate();
}

// 0x57FF80 (vtable slot 27)
// Launch: start the looping ambience that follows the tornado, then fire
// through the base with the target object dropped (the tornado wanders; it
// never homes on an object).
void CProjectileWhirlwind::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    (void)target;

    m_loopSound.SetResRef(CResRef("ARE_P23"), TRUE, TRUE);
    m_loopSound.SetFireForget(TRUE);
    m_loopSound.SetChannel(15, reinterpret_cast<DWORD>(m_pArea));
    m_loopSound.Play(m_pos.x, m_pos.y, 0, FALSE);

    IcewindCProjectileTravellingVFX::Fire(pArea, source, CGameObjectArray::INVALID_INDEX, targetPos, nHeight, nType);
}

// 0x5847B0
// Free helper shared by the wander pickers: step `distance` units from `src`
// along the (dx, dy) heading -- the heading vector is rescaled to the
// requested length on the float path (sqrt + chop), zero heading copies the
// source point.
static POINT* StepAlongHeading(POINT* pResult, const POINT* pSrc, int dx, int dy, int distance)
{
    if (dx == 0 && dy == 0) {
        *pResult = *pSrc;
        return pResult;
    }

    double scale = sqrt(static_cast<double>(distance * distance)
        / static_cast<double>(dx * dx + dy * dy));
    pResult->x = static_cast<LONG>(dx * scale + pSrc->x);
    pResult->y = static_cast<LONG>(dy * scale + pSrc->y);
    return pResult;
}

// 0x5800E0
// Pick the next wander point ~100 units away. The random path reseeds the
// CRT generator from m_wanderSeed and stores the fresh roll back, so every
// pick advances the same deterministic chain on all machines (the seed is
// what CMessageFireProjectile +0x20 replicates); bReverseFacing instead
// turns the current heading around (+8 of 16 directions). The 16-entry step
// table is the isometric direction rose.
POINT* CProjectileWhirlwind::PickWanderPoint(POINT* pResult, BOOL bReverseFacing)
{
    int direction;
    if (bReverseFacing) {
        direction = (GetDirection(CPoint(m_targetX, m_targetY)) + 8) % 16;
    } else {
        srand(m_wanderSeed);
        m_wanderSeed = rand();
        direction = m_wanderSeed % 16;
    }

    int dx, dy;
    switch (direction) {
    case 0:
        dx = 20;
        dy = 0;
        break;
    case 1:
        dx = 25;
        dy = -8;
        break;
    case 2:
        dx = 19;
        dy = -14;
        break;
    case 3:
        dx = 10;
        dy = -18;
        break;
    case 4:
        dx = 0;
        dy = -27;
        break;
    case 5:
        dx = -10;
        dy = -18;
        break;
    case 6:
        dx = -19;
        dy = -14;
        break;
    case 7:
        dx = -25;
        dy = -8;
        break;
    case 8:
        dx = -20;
        dy = 0;
        break;
    case 9:
        dx = -25;
        dy = 8;
        break;
    case 10:
        dx = -19;
        dy = 14;
        break;
    case 11:
        dx = -10;
        dy = 18;
        break;
    case 12:
        dx = 0;
        dy = 27;
        break;
    case 13:
        dx = 10;
        dy = 18;
        break;
    case 14:
        dx = 19;
        dy = 14;
        break;
    case 15:
        dx = 25;
        dy = 8;
        break;
    default:
        // Unreachable: direction is always 0..15 (the binary's default arm
        // feeds the out-pointer value as the step).
        dx = 0;
        dy = 0;
        break;
    }

    POINT pos;
    pos.x = m_pos.x;
    pos.y = m_pos.y;
    return StepAlongHeading(pResult, &pos, dx, dy, 100);
}

// 0x580270 (vtable slot 28)
// A wander leg arrived: pick the next wander point and roll the next leg
// budget.
void CProjectileWhirlwind::OnArrival()
{
    POINT nextPoint;
    PickWanderPoint(&nextPoint, FALSE);
    m_targetX = nextPoint.x;
    m_targetY = nextPoint.y;
    m_nLegBudget = rand() % 50 + 50;
}

// 0x5806C0
// Build the line-beam over the family travelling-VFX base, then arm the strike
// tracker: it owns this projectile, services every 2 ticks and re-strikes each
// victim every 33 in-range passes (gather range and beam length are stamped per
// spell by DecodeProjectile).
CProjectileLightningBolt::CProjectileLightningBolt(const CResRef& resRef)
    : IcewindCProjectileTravellingVFX(resRef)
{
    m_beamRange = 400;
    m_targetMap.m_pOwner = this;
    m_targetMap.m_servicePeriod = 2;
    m_targetMap.m_strikeInterval = 33;
}

// 0x580770 (vtable slot 0)
CProjectileLightningBolt::~CProjectileLightningBolt()
{
}

// 0x5808A0 (vtable slot 3)
// Per-tick flight for the line beam: advance the subpixel integrator straight
// toward the (overshot) target, removing on a wall, the area edge or once the
// strike tracker has spent its cap, servicing the tracker each tick so it rakes
// everything it passes, and arriving when it reaches the end of the line.
void CProjectileLightningBolt::AIUpdate()
{
    // Frozen by Time Stop unless this is the freezing caster's own shot.
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    // Strike tracker reached its total cap: remove and free.
    if (m_targetMap.m_bDone == 1) {
        RemoveFromArea();
        if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(
                m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE)
            == CGameObjectArray::SUCCESS) {
            delete this;
        }
        return;
    }

    // Grid square before advancing (a wall check only fires on a crossing).
    CPoint oldGrid(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX, m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);

    // Advance the subpixel integrator (y carries the 3/4 isometric scale).
    m_posAccumX += m_stepX;
    m_posAccumY += m_stepY;
    m_pos.x = m_posAccumX >> CGameSprite::EXACT_SCALE;
    m_pos.y = ((m_posAccumY * 3) / 4) >> CGameSprite::EXACT_SCALE;

    // Off the area: remove and free.
    if (m_pos.x < 0 || m_pos.y < 0
        || m_pArea->GetInfinity()->nAreaX <= m_pos.x
        || m_pArea->GetInfinity()->nAreaY <= m_pos.y) {
        RemoveFromArea();
        if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(
                m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE)
            == CGameObjectArray::SUCCESS) {
            delete this;
        }
        return;
    }

    // Into a fresh impassable square: the beam stops dead (no bounce). Remove.
    CPoint newGrid(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX, m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);
    if (oldGrid.x != newGrid.x || oldGrid.y != newGrid.y) {
        SHORT nTableIndex;
        if (m_pArea->m_search.GetLOSCost(newGrid, m_terrainTable, nTableIndex, FALSE)
            == CPathSearch::COST_IMPASSABLE) {
            RemoveFromArea();
            if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(
                    m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE)
                == CGameObjectArray::SUCCESS) {
                delete this;
            }
            return;
        }
    }

    // Service the strike tracker and keep the loop sound on the beam head.
    m_targetMap.Service();
    m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);

    // Reached the line's end (+/-32 x, +/-24 y): deliver; else keep flying.
    if (m_targetX < m_pos.x + 0x20 && m_pos.x - 0x20 < m_targetX
        && m_targetY < m_pos.y + 0x18 && m_pos.y - 0x18 < m_targetY) {
        OnArrival();
    } else {
        IcewindCProjectileTravellingVFX::AIUpdate();
    }
}

// 0x580B30 (vtable slot 27)
// Push the picked target point out to m_beamRange past it along the caster->
// target heading, so the bolt overshoots its mark and rakes the full line, then
// launch through the base.
void CProjectileLightningBolt::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    CGameObject* pSource;
    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(
            source, CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSource), INFINITE)
        == CGameObjectArray::SUCCESS) {
        CPoint& casterPos = pSource->GetPos();
        CPoint endPoint = IcewindMisc::ScaleToCircle(targetPos,
            targetPos.x - casterPos.x, targetPos.y - casterPos.y, m_beamRange);
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
            source, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        IcewindCProjectileTravellingVFX::Fire(pArea, source, target, endPoint, nHeight, nType);
    }
}

// 0x579B40
// Builds the cone carrier: an invisible MMissiT cell plus the cone geometry
// constants (length 200, radii 350/25, the segment divisors 20/35/45, a 15-tick
// lifetime that pulses every 5 ticks). The cone's own BAM arrives as coneBam.
//
// The base subobject is built through the cone-specific cell-less emission at
// address 0x577E40 (per the family modelling licence), represented here as the
// ordinary base ctor, whose carrier cell is then swapped for MMissiT below.
CProjectileCone::CProjectileCone(const CResRef& cellBam, const CResRef& coneBam)
    : IcewindCProjectileTravellingVFX(cellBam)
{
    m_coneLength = 200;
    m_outerRadius = 0x15E;
    m_coneRadius = 0x19;
    field_2DE = coneBam.GetResRef()[0];

    // Copy coneBam into the reference-counted name buffer the way the original
    // does: clear (0x448D50), then reserve (0x44BC20 -> grow 0x44BE10/0x44BE90)
    // with the strlen and memcpy inlined at 0x579BA8/0x579BD1. The buffer is one
    // heap block whose first byte is a share count (0 == sole owner) and whose
    // character data starts at block + 1. An empty name allocates nothing (the
    // reserve early-outs at 0x44BC20 and the copy is skipped at 0x579BBF).
    const char* coneName = (const char*)coneBam.GetResRef();
    LONG nameLen = (LONG)strlen(coneName);
    if (nameLen != 0) {
        LONG nameCap = nameLen | 0x1F;          // 0x44BE10 rounds capacity up
        char* block = new char[nameCap + 2];    // share byte + data + terminator
        block[0] = 0;                           // 0x44BE90: this is the sole owner
        m_pName = block + 1;
        memcpy(m_pName, coneName, nameLen);      // 0x579BD1 rep movs
        m_pName[nameLen] = '\0';                 // 0x579BE4
        m_nameLen = nameLen;
        m_nameCap = nameCap;
    } else {
        m_pName = NULL;
        m_nameLen = 0;
        m_nameCap = 0;
    }

    field_2EE = 1;
    m_segmentStep = 0x14;
    field_306 = 0x23;
    field_30A = 0x2D;
    m_duration = 0xF;
    m_pulsePeriod = 5;
    field_316 = 0;
    field_317 = 1;
    m_segCount = 0;
    m_tickCount = 0;
    m_bFinishing = 0;
    m_nHeight = 0;
    m_nType = 0;
    m_bMirrorNorth = 0;
    m_bMirrorEast = 1;

    // Carrier cell: an empty cell name selects the invisible MMissiT carrier; a
    // real name uses that BAM. The cell the base ctor built from cellBam is
    // replaced either way (the binary's cell-less base build skips it).
    delete m_pVidCell;
    if (cellBam.GetResRef()[0] == '\0') {
        m_pVidCell = new CVidCell(CResRef("MMissiT"), FALSE);
        m_visible = 0;
    } else {
        m_pVidCell = new CVidCell(cellBam, FALSE);
        m_visible = 1;
    }
    m_bHasHeight = TRUE;
}

// 0x579DE0 (vtable slot 0; the deleting thunk 0x579DC0 wraps this)
// Frees the cone edge-point buffer and releases the refcounted name; the base
// destructor then tears down the carrier cell and visual effect.
CProjectileCone::~CProjectileCone()
{
    // m_edgePoints (std::vector) releases its buffer automatically (the original
    // frees it explicitly here). Release the reference-counted name the way the
    // original inlines 0x448D50 at 0x579E09: if this object holds the only share
    // (count 0, or the 0xFF sentinel) free the block, otherwise drop the share
    // count by one. The block starts one byte before the character data.
    if (m_pName != NULL) {
        char* block = m_pName - 1;
        char count = block[0];
        if (count == 0 || count == (char)0xFF) {
            delete[] block;                     // 0x7FC984: sole owner -> free
        } else {
            block[0] = count - 1;               // shared -> drop the count
        }
    }
    m_pName = NULL;
    m_nameLen = 0;
    m_nameCap = 0;
}

// 0x579E50 (vtable slot 3)
// Cone lifetime tick: count up; once past m_duration a finishing cone removes
// itself and is freed; otherwise it pulses a fresh layer every m_pulsePeriod
// ticks. While not finishing it also runs the base flight tick.
void CProjectileCone::AIUpdate()
{
    m_tickCount++;
    if (m_duration < m_tickCount) {
        if (m_bFinishing == 1) {
            RemoveFromArea();
            if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(
                    m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE)
                == CGameObjectArray::SUCCESS) {
                delete this;
            }
            return;
        }
    } else if (m_tickCount % m_pulsePeriod == 1) {
        Pulse();
    }
    if (m_bFinishing == 0) {
        IcewindCProjectileTravellingVFX::AIUpdate();
    }
}

// 0x579EF0 (vtable slot 27)
// Resolve the caster position, then build the cone's geometry from the caster
// toward the target point: the segment count, the four corner points and the
// arc-centre point. Two near corners sit +/- m_coneRadius perpendicular to the
// aim at the caster; the arc centre sits m_outerRadius along the aim; the two
// far corners sit +/- (m_coneRadius + m_coneLength/2) perpendicular at the
// (m_coneRadius + m_outerRadius) point along the aim. Each coordinate scales a
// direction component by radius/distance (truncated to LONG, like the original
// __ftol). Then launch the invisible carrier through the base.
void CProjectileCone::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
    m_nHeight = nHeight;
    m_nType = nType;
    m_sourceId = source;
    m_pArea = pArea;

    // Caster must exist; GetStart re-shares it to read the
    // facing-adjusted launch point into m_casterPos.
    CGameObject* pCaster;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceId,
            CGameObjectArray::THREAD_ASYNCH, &pCaster, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    GetStart(m_sourceId, m_casterPos, TRUE);
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceId,
        CGameObjectArray::THREAD_ASYNCH, INFINITE);

    m_segCount = (m_coneLength / m_segmentStep) / 2;

    int dx = targetPos.x - m_casterPos.x;
    int dy = targetPos.y - m_casterPos.y;
    double distSq = static_cast<double>(dx * dx + dy * dy);

    // Near corners: +/- m_coneRadius perpendicular to the aim, at the caster.
    double ratioNear = sqrt(static_cast<double>(m_coneRadius * m_coneRadius) / distSq);
    m_edge0.x = static_cast<LONG>(static_cast<double>(m_casterPos.x) + static_cast<double>(-dy) * ratioNear);
    m_edge0.y = static_cast<LONG>(static_cast<double>(m_casterPos.y) + static_cast<double>(dx) * ratioNear);
    m_edge1.x = static_cast<LONG>(static_cast<double>(m_casterPos.x) + static_cast<double>(dy) * ratioNear);
    m_edge1.y = static_cast<LONG>(static_cast<double>(m_casterPos.y) + static_cast<double>(-dx) * ratioNear);

    // Arc centre: m_outerRadius along the aim.
    double ratioCentre = sqrt(static_cast<double>(m_outerRadius * m_outerRadius) / distSq);
    m_center.x = static_cast<LONG>(static_cast<double>(m_casterPos.x) + static_cast<double>(dx) * ratioCentre);
    m_center.y = static_cast<LONG>(static_cast<double>(m_casterPos.y) + static_cast<double>(dy) * ratioCentre);

    // Far corners: from the (m_coneRadius + m_outerRadius) point along the aim,
    // +/- (m_coneRadius + m_coneLength/2) perpendicular.
    LONG farSum = m_coneRadius + m_outerRadius;
    double ratioFarBase = sqrt(static_cast<double>(farSum * farSum) / distSq);
    LONG farBaseX = static_cast<LONG>(static_cast<double>(m_casterPos.x) + static_cast<double>(dx) * ratioFarBase);
    LONG farBaseY = static_cast<LONG>(static_cast<double>(m_casterPos.y) + static_cast<double>(dy) * ratioFarBase);

    LONG farHalf = m_coneRadius + m_coneLength / 2;
    double ratioFar = sqrt(static_cast<double>(farHalf * farHalf) / distSq);
    m_edge2.x = static_cast<LONG>(static_cast<double>(farBaseX) + static_cast<double>(dy) * ratioFar);
    m_edge2.y = static_cast<LONG>(static_cast<double>(farBaseY) + static_cast<double>(-dx) * ratioFar);
    m_edge3.x = static_cast<LONG>(static_cast<double>(farBaseX) + static_cast<double>(-dy) * ratioFar);
    m_edge3.y = static_cast<LONG>(static_cast<double>(farBaseY) + static_cast<double>(dx) * ratioFar);

    // Fan the cone arc: sweep m_segCount segments either side of the centre,
    // each offset m_segmentStep*|i| perpendicular to the aim (the edge0 side for
    // i < 0, the edge1 side for i > 0), then append the centre point itself.
    for (int i = -m_segCount; i < m_segCount; ++i) {
        CPoint pt;
        if (i < 0) {
            int off = -(m_segmentStep * i);
            double ratio = sqrt(static_cast<double>(off * off) / distSq);
            pt.x = static_cast<LONG>(static_cast<double>(m_center.x) + static_cast<double>(-dy) * ratio);
            pt.y = static_cast<LONG>(static_cast<double>(m_center.y) + static_cast<double>(dx) * ratio);
        } else if (i == 0) {
            pt = m_center;
        } else {
            int off = m_segmentStep * i;
            double ratio = sqrt(static_cast<double>(off * off) / distSq);
            pt.x = static_cast<LONG>(static_cast<double>(m_center.x) + static_cast<double>(dy) * ratio);
            pt.y = static_cast<LONG>(static_cast<double>(m_center.y) + static_cast<double>(-dx) * ratio);
        }
        m_edgePoints.insert(m_edgePoints.end(), 1, pt);
    }
    m_edgePoints.insert(m_edgePoints.end(), 1, m_center);

    IcewindCProjectileTravellingVFX::Fire(pArea, source, target, targetPos, nHeight, nType);
}

// 0x57A530 (vtable slot 28)
// The carrier has reached the target point: notify the callback projectile,
// play the arrival sound, remove the linked target projectile, then strike
// (DeliverEffects paints/strikes the cone and flags it finishing).
void CProjectileCone::OnArrival()
{
    if (m_callBackProjectile != CGameObjectArray::INVALID_INDEX) {
        CGameObject* pObject;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(
                m_callBackProjectile, CGameObjectArray::THREAD_ASYNCH, &pObject, INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }
        static_cast<CProjectile*>(pObject)->CallBack();
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_callBackProjectile,
            CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }

    PlaySound(m_arrivalSoundRef, m_loopArrivalSound, TRUE);

    if (m_nTargetId != CGameObjectArray::INVALID_INDEX) {
        CGameObject* pTarget;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(
                m_nTargetId, CGameObjectArray::THREAD_ASYNCH, &pTarget, INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
        if (rc == CGameObjectArray::SUCCESS) {
            static_cast<CProjectile*>(pTarget)->RemoveSelf();
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_nTargetId,
                CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    }

    DeliverEffects();
}

// 0x57A670 (vtable slot 30)
// Strike everything inside the cone. Gather every sprite inside the four-corner
// cone polygon, then for each one (other than the caster) that is an AI object
// in line of sight from the caster, queue a copy of each carried effect against
// it. Finally flag the cone finishing so AIUpdate tears it down.
void CProjectileCone::DeliverEffects()
{
    // Bounding box of the four cone corners (contiguous CPoints from m_edge0).
    CRect rBounding;
    rBounding.left = rBounding.right = m_edge0.x;
    rBounding.top = rBounding.bottom = m_edge0.y;
    const CPoint* pCorners = &m_edge0;
    for (int e = 1; e < 4; ++e) {
        if (pCorners[e].x < rBounding.left) rBounding.left = pCorners[e].x;
        if (rBounding.right < pCorners[e].x) rBounding.right = pCorners[e].x;
        if (pCorners[e].y < rBounding.top) rBounding.top = pCorners[e].y;
        if (rBounding.bottom < pCorners[e].y) rBounding.bottom = pCorners[e].y;
    }

    CTypedPtrList<CPtrList, LONG*> targets;
    m_pArea->GetAllInPoly(rBounding, &m_edge0, 4, CAIObjectType::ANYONE,
        m_pArea->m_visibleTerrainTable, targets, 1);

    POSITION pos = targets.GetHeadPosition();
    while (pos != NULL) {
        LONG nTargetId = reinterpret_cast<LONG>(targets.GetNext(pos));

        CGameObject* pObject;
        if (nTargetId != m_sourceId
            && g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nTargetId,
                   CGameObjectArray::THREAD_ASYNCH, &pObject, INFINITE)
               == CGameObjectArray::SUCCESS) {
            if (pObject->GetObjectType() & CGameObject::TYPE_AIBASE) {
                if (m_pArea->CheckLOS(m_casterPos, pObject->GetPos(), m_terrainTable, FALSE)) {
                    POSITION posEffect = m_effectList.GetHeadPosition();
                    while (posEffect != NULL) {
                        CGameEffect* pEffect = m_effectList.GetNext(posEffect);
                        CMessage* msg = new CMessageAddEffect(pEffect->Copy(), m_sourceId, nTargetId);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
                    }
                }
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nTargetId,
                CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    }

    m_bFinishing = 1;
}

// (inlined into CProjectileCone::Pulse) -- a plain travelling-VFX leaf carrying
// the cone BAM; only the vtable distinguishes it from the base.
CProjectileConePulseVisual::CProjectileConePulseVisual(const CResRef& resRef)
    : IcewindCProjectileTravellingVFX(resRef)
{
}

// 0x579B10 (vtable slot 0)
CProjectileConePulseVisual::~CProjectileConePulseVisual()
{
}

// 0x57A970 (vtable slot 34; the BG2 CProjectileConeOfCold::DoLayers)
// Emit one cone pulse: fire a spray visual along every fan point. Each visual
// flies the cone BAM, takes a copy of the cone's visual effect, a random launch
// frame, and a randomised fraction of the cone velocity, and is launched (no
// target) toward its fan point. Inert until the cone carries a BAM name.
void CProjectileCone::Pulse()
{
    if (m_nameLen == 0) {
        return;
    }

    for (std::vector<CPoint>::iterator it = m_edgePoints.begin(); it != m_edgePoints.end(); ++it) {
        CProjectileConePulseVisual* pVisual =
            new CProjectileConePulseVisual(CResRef(m_pName != NULL ? m_pName : ""));
        if (pVisual != NULL) {
            pVisual->m_visualEffect = m_visualEffect;
            pVisual->m_bHasHeight = TRUE;
            if (field_316 != 1) {
                BYTE nFrames = pVisual->m_pVidCell->GetSequenceLength(0, FALSE);
                if (nFrames == 0) {
                    pVisual->m_pVidCell->FrameSet(0);
                } else {
                    pVisual->m_pVidCell->FrameSet(rand() % nFrames);
                }
            }
        }
        pVisual->m_bMirrorNorth = m_bMirrorNorth;
        pVisual->m_dirCount = static_cast<SHORT>(field_2EE);
        int nSpread = field_30A != 0 ? rand() % field_30A : 0;
        pVisual->m_velocity = static_cast<SHORT>((field_306 + nSpread) * m_velocity / 100);
        pVisual->Fire(m_pArea, m_sourceId, CGameObjectArray::INVALID_INDEX, *it, m_nHeight, m_nType);
    }
}

// 0x56FDC0. Spell-hit emission-slot descriptor ctor. The string slots and the
// IcewindCVisualEffect default-construct; the body clears the cache fields and
// stamps the 0x7FFFFFFF sentinel. The binary stamps field_0/field_10 from an
// uninitialised stack byte (a compiler artifact); we clear them.
IcewindCSpellHitEmission::IcewindCSpellHitEmission()
{
    m_resref0Flags = 0;
    m_resref0 = NULL;
    m_resref0Len = 0;
    m_resref0Cap = 0;
    m_resref1Flags = 0;
    m_resref1 = NULL;
    m_resref1Len = 0;
    m_resref1Cap = 0;
    m_animMode = 0;
    m_maxMovingSpawn = 0x7FFFFFFF;
}

// 0x56FE30. The ranged emission-slot descriptor ctor: same prefix clear, then
// the trailing geometry/timing defaults (250 / 6 / 30).
IcewindCSpellHitEmissionRanged::IcewindCSpellHitEmissionRanged()
{
    m_resref0Flags = 0;
    m_resref0 = NULL;
    m_resref0Len = 0;
    m_resref0Cap = 0;
    m_resref1Flags = 0;
    m_resref1 = NULL;
    m_resref1Len = 0;
    m_resref1Cap = 0;
    m_cellPool = NULL;
    m_lastCellIndex = 0;
    m_respawnFlag = 0;
    m_animMode = 0;
    m_animFlag36 = 0;
    m_densityBase = 250;
    m_emitPeriod = 6;
    m_densityRampDiv = 30;
    m_cloudFlag = 0;
}

// =============================================================================
// IcewindCSpellHitVisual -- the on-ground detonation visual (SCAFFOLD).
// Layout is recovered (see CProjectile.h); the bodies below are faithful stubs.
// The base CGameObject and the typed sub-objects (m_cell, m_palette, m_sound,
// m_visualEffect) are still constructed/destroyed; the real work -- BAM load,
// radial velocity table, CGameObjectArray::Add + AddToArea, frame advance and
// render -- is recovered in a later pass.
// =============================================================================

// 0x56BF30. Builds the on-ground detonation animation: loads the explosion BAM
// into m_cell from the first emission slot, copies the other two emission slots
// in, then rasterises two quarter-circle arcs (CVidMode::GetEllipseArcPixelList)
// and stores, for every cell of the radial fan, the velocity vector normalised
// to the launch speed -- replicated across the four quadrants -- before
// registering the object and adding it to the area.
IcewindCSpellHitVisual::IcewindCSpellHitVisual(const IcewindCSpellHitEmission& emission0,
    const IcewindCSpellHitEmission& emission1, const IcewindCSpellHitEmissionRanged& emission2,
    CGameArea* pArea, const CPoint& pos, SHORT nRange, BYTE nVelocity, BYTE a8, SHORT nDuration)
    : m_palette(1 /* DAT_0085E84A */)
{
    m_emitCooldown = 0;
    m_bamLoaded = 0;
    m_age = 0;
    m_duration = nDuration;
    m_movingSpawnCount = 0;
    m_frameCount = static_cast<BYTE>((nRange - 1) / nVelocity) + 1;
    m_collision = a8;

    // Load the detonation BAM (CResCell, type 1000) and its header (CResCellHeader,
    // type 1100) into m_cell from the first emission slot's resref.
    if (emission0.m_resref0Len == 0) {
        m_bamLoaded = 0;
    } else {
        m_cell.SetResRef(CResRef(emission0.m_resref0), FALSE, TRUE, TRUE);
        m_bamLoaded = 1;
    }

    m_visualEffect = emission0.m_visualEffect;
    m_emission1 = emission1;
    m_emission2 = emission2;

    m_cell.SequenceSet(0);
    m_cell.FrameSet(0);
    field_7E = 0;
    field_80 = 0;
    m_fanCells = NULL;

    // Allocate the per-cell pixel-run buffer sized for the full radius.
    int nRadius = (nRange - 1) / 16;
    m_coverHalfW = nRadius + 1;
    m_coverHalfH = nRadius + 1;
    m_coverageMap = malloc((nRadius + 2 + nRadius + 1) * (nRadius + 2 + nRadius + 1));
    if (m_coverageMap == NULL) {
        return;
    }

    // Rasterise the two arcs (half-radius major/minor axes); each call appends a
    // run-length terminator and returns the run count.
    int nHalf = (nRange - 1) / 32 + 1;
    m_coverHalfW = nHalf;
    m_coverHalfH = nHalf;
    CBaldurEngine* pEngine = g_pBaldurChitin->GetActiveEngine();
    int nRun1 = pEngine->pVidMode->GetEllipseArcPixelList(m_coverHalfW, nHalf,
        static_cast<BYTE*>(m_coverageMap));
    m_arcLen1 = nRun1 + 1;
    static_cast<BYTE*>(m_coverageMap)[nRun1] = 1;

    pEngine = g_pBaldurChitin->GetActiveEngine();
    int nRun2 = pEngine->pVidMode->GetEllipseArcPixelList(m_coverHalfH, m_coverHalfW,
        static_cast<BYTE*>(m_coverageMap) + m_arcLen1);
    m_arcLen2 = nRun2 + 1;
    static_cast<BYTE*>(m_coverageMap)[m_arcLen1 + nRun2] = 1;

    m_coverHalfW = (nRange - 1) / 16 + 1;
    m_coverHalfH = m_coverHalfW;

    // Velocity table: 16 bytes per cell, four quadrants.
    m_fanCells = malloc((m_arcLen2 + m_arcLen1) * 0x40);
    if (m_fanCells == NULL) {
        return;
    }
    m_fanVel = malloc((m_arcLen2 + m_arcLen1) * 4);
    if (m_fanVel == NULL) {
        return;
    }
    memset(m_fanVel, 0, (m_arcLen2 + m_arcLen1) * 4);

    LONG* pVel = static_cast<LONG*>(m_fanCells);
    BYTE* pFlag = static_cast<BYTE*>(m_fanVel);
    BYTE* pArc = static_cast<BYTE*>(m_coverageMap);
    LONG nStride = m_arcLen1 + m_arcLen2;

    LONG nCenterX = pos.x;
    LONG nIsoY = (pos.y << 2) / 3;
    LONG nCursorX = nCenterX;
    LONG nCursorY = nRange + nIsoY;

    for (SHORT i = 0; i < m_arcLen1; ++i) {
        LONG dx = nCursorX - nCenterX;
        LONG dy = nCursorY - nIsoY;
        LONG dist = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)));
        LONG i0 = i;
        LONG i1 = nStride + i;
        LONG i2 = i + nStride * 2;
        LONG i3 = i + nStride * 3;
        pVel[i0 * 4 + 1] = 0;
        pVel[i1 * 4 + 1] = 0;
        pVel[i2 * 4 + 1] = 0;
        pVel[i3 * 4 + 1] = 0;
        pVel[i0 * 4] = 0;
        pVel[i1 * 4] = 0;
        pVel[i2 * 4] = 0;
        pVel[i3 * 4] = 0;
        LONG vx = (dx << 10) * nVelocity / dist;
        pVel[i0 * 4 + 3] = vx;
        pVel[i1 * 4 + 3] = vx;
        if (vx == 0) {
            pFlag[i2] = 2;
            pFlag[i3] = 2;
        }
        pVel[i2 * 4 + 3] = -vx;
        pVel[i3 * 4 + 3] = -vx;
        LONG vy = (dy << 10) * nVelocity / dist;
        pVel[i0 * 4 + 2] = vy;
        pVel[i1 * 4 + 2] = -vy;
        pVel[i2 * 4 + 2] = vy;
        pVel[i3 * 4 + 2] = -vy;
        nCursorX += 16 * 2;
        nCursorY -= pArc[i] * 12 * 2;
    }

    nCursorX = nRange + nCenterX;
    nCursorY = nIsoY;
    for (SHORT i = 0; i < m_arcLen2; ++i) {
        LONG dy = nCursorY - nIsoY;
        LONG dx = nCursorX - nCenterX;
        LONG dist = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)));
        LONG j0 = m_arcLen1 + i;
        LONG j1 = m_arcLen2 + m_arcLen1 * 2 + i;
        LONG j2 = i + nStride * 2 + m_arcLen1;
        LONG j3 = i + nStride * 3 + m_arcLen1;
        pVel[j0 * 4 + 1] = 0;
        pVel[j1 * 4 + 1] = 0;
        pVel[j2 * 4 + 1] = 0;
        pVel[j3 * 4 + 1] = 0;
        pVel[j0 * 4] = 0;
        pVel[j1 * 4] = 0;
        pVel[j2 * 4] = 0;
        pVel[j3 * 4] = 0;
        LONG vx = (dx << 10) * nVelocity / dist;
        pVel[j0 * 4 + 3] = vx;
        pVel[j1 * 4 + 3] = vx;
        pVel[j2 * 4 + 3] = -vx;
        pVel[j3 * 4 + 3] = -vx;
        LONG vy = (dy << 10) * nVelocity / dist;
        pVel[j0 * 4 + 2] = vy;
        pVel[j1 * 4 + 2] = -vy;
        pVel[j2 * 4 + 2] = vy;
        pVel[j3 * 4 + 2] = -vy;
        if (pVel[i * 4 + 2] == 0) {
            pFlag[j1] = 2;
            pFlag[j3] = 2;
        }
        nCursorX -= pArc[m_arcLen1 + i] * 16 * 2;
        nCursorY += 12 * 2;
    }

    memcpy(m_terrainTable, CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE, sizeof(m_terrainTable));

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE)
        == CGameObjectArray::SUCCESS) {
        AddToArea(pArea, pos, 0, 0 /* DAT_0084C50C */);
    }
}

// 0x56CEE0 (vtable slot 0 -- MSVC scalar-deleting wrapper; body at 0x56CF00).
// Frees the three malloc'd fan buffers; the CVidCell, CVidPalette, CSound, the
// IcewindCVisualEffect, the two emission descriptors and the CGameObject base all
// destruct automatically.
IcewindCSpellHitVisual::~IcewindCSpellHitVisual()
{
    free(m_coverageMap);
    free(m_fanCells);
    free(m_fanVel);
}

// 0x56D0A0 (vtable slot 3). Each tick advances the detonation and expands the
// radial fan: every live cell drifts by its velocity, bounces off / dies on walls
// (CSearchBitmap::GetLOSCost) per the mode byte, and spawns IcewindCSpellHit
// particles -- stationary m_emission2 cells at an age-ramped probability (recorded
// into the shared cell pool), and moving m_emission1 cells once per fresh coverage
// pixel. The visual expires once the fan has settled and the BAM sequence ends.
void IcewindCSpellHitVisual::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    BOOL bSettled = TRUE;
    m_age++;
    m_frameCount--;

    if (m_bamLoaded == 1) {
        if (m_frameCount == 0) {
            if (m_cell.IsEndOfSequence(FALSE)) {
                RemoveFromArea();
                return;
            }
            m_frameCount++;
            m_cell.FrameAdvance();
            return;
        }
    } else if (m_frameCount == 0) {
        RemoveFromArea();
        return;
    }

    // Reset the per-cell coverage map.
    memset(m_coverageMap, 0, (m_coverHalfH * 2 + 1) * (m_coverHalfW * 2 + 1));

    int nCellCount = (m_arcLen1 + m_arcLen2) * 4;
    if (nCellCount > 0) {
        SHORT nIndex = 0;
        int i = 0;
        do {
            BYTE* pFlags = static_cast<BYTE*>(m_fanVel);
            if (pFlags[i] != 2) {
                LONG* pCell = reinterpret_cast<LONG*>(static_cast<BYTE*>(m_fanCells) + i * 0x10);
                pCell[1] += pCell[3];   // pos.y += vel.y
                pCell[0] += pCell[2];   // pos.x += vel.x

                int nTileY = (((pCell[0] * 3 / 4) >> CGameSprite::EXACT_SCALE) + m_pos.y)
                    / CPathSearch::GRID_SQUARE_SIZEY;
                int nTileX = ((pCell[1] >> CGameSprite::EXACT_SCALE) + m_pos.x)
                    / CPathSearch::GRID_SQUARE_SIZEX;

                SHORT nTableIndex;
                BYTE cost = m_pArea->m_search.GetLOSCost(CPoint(nTileX, nTileY),
                    m_terrainTable, nTableIndex, FALSE);

                BOOL bRender = TRUE;
                if (cost == CPathSearch::COST_IMPASSABLE) {
                    if (m_collision == CGameTemporal::COLLISION_DESTROY) {
                        pFlags[i] = 2;
                    }
                    if (m_collision == CGameTemporal::COLLISION_REBOUND) {
                        // Reverse the velocity on whichever axis crossed the wall.
                        if ((((pCell[1] - pCell[3]) >> CGameSprite::EXACT_SCALE) + m_pos.x)
                                / CPathSearch::GRID_SQUARE_SIZEX != nTileX) {
                            pCell[1] += pCell[3] * -2;
                            pCell[3] = -pCell[3];
                        }
                        if ((((pCell[0] - pCell[2]) * 3 / 4 >> CGameSprite::EXACT_SCALE) + m_pos.y)
                                / CPathSearch::GRID_SQUARE_SIZEY != nTileY) {
                            pCell[0] += pCell[2] * -2;
                            pCell[2] = -pCell[2];
                        }
                    } else {
                        bRender = FALSE;
                        if (pFlags[i] == 0) {
                            bSettled = FALSE;
                        }
                    }
                }

                if (bRender) {
                    int nPixIdx = (((pCell[1] / CPathSearch::GRID_SQUARE_SIZEX) >> CGameSprite::EXACT_SCALE) + m_coverHalfW)
                        + (((pCell[0] / CPathSearch::GRID_SQUARE_SIZEX) >> CGameSprite::EXACT_SCALE) + m_coverHalfH)
                          * (m_coverHalfW * 2 + 1);

                    // Stationary m_emission2 spawn, gated by a cooldown and a
                    // density probability that ramps with age.
                    if (m_emission2.m_resref0Len != 0) {
                        if (m_emitCooldown < 1) {
                            int nThreshold = m_age * m_age / m_emission2.m_densityRampDiv + m_emission2.m_densityBase;
                            if (m_frameCount < 2) {
                                nThreshold *= 2;
                            }
                            CPoint ptSpawn((m_pos.x << CGameSprite::EXACT_SCALE) + pCell[1],
                                (m_pos.y << CGameSprite::EXACT_SCALE) + pCell[0] * 3 / 4);
                            int nRand = rand() % 10000;
                            IcewindCSpellHitCell cell = { ptSpawn.x, ptSpawn.y, 0 };
                            if (nThreshold < nRand) {
                                m_emission2.m_cellPool->m_cells.push_back(cell);
                            } else {
                                cell.flag = 1;
                                m_emission2.m_cellPool->m_cells.push_back(cell);
                                m_emission2.m_lastCellIndex =
                                    static_cast<INT>(m_emission2.m_cellPool->m_cells.size()) - 1;
                                CPoint ptVel(0, 0);
                                new IcewindCSpellHitParticle(m_emission2, m_pArea, ptSpawn, 0, ptVel,
                                    static_cast<SHORT>(m_frameCount * 2 + m_duration), 0, m_collision);
                                m_emitCooldown = m_emission2.m_emitPeriod;
                            }
                        } else {
                            m_emitCooldown--;
                        }
                    }

                    // Moving m_emission1 spawn, once per freshly-covered pixel.
                    BYTE* pCoverage = static_cast<BYTE*>(m_coverageMap);
                    BYTE nCount = pCoverage[nPixIdx];
                    pCoverage[nPixIdx] = nCount + 1;
                    if (nCount == 0 && pFlags[i] == 0) {
                        if (m_emission1.m_resref0Len != 0 && ++m_movingSpawnCount <= m_emission1.m_maxMovingSpawn) {
                            CPoint ptSpawn((m_pos.x << CGameSprite::EXACT_SCALE) + pCell[1],
                                (m_pos.y << CGameSprite::EXACT_SCALE) + pCell[0] * 3 / 4);
                            CPoint ptVel(pCell[3], pCell[2] * 3 / 4);
                            new IcewindCSpellHitParticle(m_emission1, m_pArea, ptSpawn, 0, ptVel,
                                static_cast<SHORT>(m_frameCount), 0, m_collision);
                        }
                        pFlags[i] = 1;
                    } else if (pFlags[i] == 0) {
                        bSettled = FALSE;
                    }
                }
            }
            nIndex++;
            i = static_cast<SHORT>(nIndex);
        } while (i < nCellCount);
    }

    if (bSettled) {
        if (m_bamLoaded == 1) {
            if (m_cell.IsEndOfSequence(FALSE)) {
                RemoveFromArea();
            }
        } else {
            RemoveFromArea();
        }
    }

    if (m_bamLoaded == 1 && !m_cell.IsEndOfSequence(FALSE)) {
        m_cell.FrameAdvance();
    }
}

// 0x56D9B0 (vtable slot 18). Removes the visual from the area and the global
// object array, then deletes itself (same shape as the particle's RemoveFromArea).
void IcewindCSpellHitVisual::RemoveFromArea()
{
    CGameObject::RemoveFromArea();

    BYTE nResult = g_pBaldurChitin->GetObjectGame()->m_cObjectArray.Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH, NULL, -1);
    if (nResult != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\... __LINE__: 0x1ED
        UTIL_ASSERT(FALSE);
        return;
    }

    delete this;
}

// 0x56D730 (vtable slot 19). Draws the detonation BAM's current frame at the
// impact point through the CInfinity FX pipeline, gated on tile visibility, a
// loaded BAM (m_bamLoaded) and the sequence not having finished.
void IcewindCSpellHitVisual::Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface)
{
    CRect rFX;
    CRect rGCBounds;
    CSize frameSize;
    CPoint ptReference;
    CPoint newPos;
    CInfinity* pInfinity;

    (void)pArea;
    (void)pVidMode;   // binary asserts pVidMode != NULL here; UtilAssert omitted (it kills the game)

    // Skip if the impact tile is off-screen or no detonation BAM was loaded. The
    // point is clamped to the area bounds before the visibility lookup.
    LONG nVisX = m_pos.x < 0 ? 0 : m_pos.x;
    LONG nAreaW = m_pArea->GetInfinity()->nAreaX;
    if (nVisX > nAreaW - 1) {
        nVisX = nAreaW - 1;
    }
    LONG nVisY = m_pos.y < 0 ? 0 : m_pos.y;
    LONG nAreaH = m_pArea->GetInfinity()->nAreaY;
    if (nVisY > nAreaH - 1) {
        nVisY = nAreaH - 1;
    }
    if (!m_pArea->m_visibility.IsTileVisible(m_pArea->m_visibility.PointToTile(CPoint(nVisX, nVisY)))
        || m_bamLoaded == 0) {
        return;
    }
    if (m_cell.IsEndOfSequence(FALSE)) {
        return;
    }

    m_cell.GetCurrentCenterPoint(ptReference, FALSE);
    m_cell.GetCurrentFrameSize(frameSize, FALSE);
    rFX.SetRect(0, 0, frameSize.cx, frameSize.cy);

    newPos.x = m_pos.x;
    newPos.y = m_pArea->GetHeightOffset(m_pos, m_listType) + m_pos.y;

    rGCBounds.left = newPos.x - ptReference.x;
    rGCBounds.top = newPos.y - ptReference.y;
    rGCBounds.right = rGCBounds.left + rFX.Width();
    rGCBounds.bottom = rGCBounds.top + rFX.Height();

    DWORD dwPrepFlags = m_visualEffect.m_dwFlags | CInfinity::FXPREP_COPYFROMBACK;

    pInfinity = m_pArea->GetInfinity();
    pInfinity->FXPrep(rFX, dwPrepFlags, nSurface, newPos, ptReference);
    if (pInfinity->FXLock(rFX, dwPrepFlags)) {
        pInfinity->FXRender(&m_cell, ptReference.x, ptReference.y, m_visualEffect.m_dwFlags, 0);
        pInfinity->FXRenderClippingPolys(newPos.x, newPos.y - m_posZ, m_posZ, ptReference, rGCBounds,
            FALSE, m_visualEffect.m_dwFlags);
        pInfinity->FXUnlock(dwPrepFlags, NULL, CPoint(0, 0));
        pInfinity->FXBltFrom(nSurface, rFX, newPos.x, newPos.y, ptReference.x, ptReference.y,
            m_visualEffect.m_dwFlags | 0x1);
    }
}

// 0x56DF00. The m_emission1 sibling of the 0x56E280 ctor: same particle setup
// from the plainer IcewindCSpellHitEmission, which carries no shared cell object
// (m_cellPool..m_hasCloud are cleared rather than copied from the descriptor).
IcewindCSpellHitParticle::IcewindCSpellHitParticle(const IcewindCSpellHitEmission& descriptor,
    CGameArea* pArea, const CPoint& pos, int a5, const CPoint& velocity, SHORT a7, BYTE mode8,
    BYTE mode9)
{
    m_cellPool = NULL;
    m_cellIndex = 0;
    m_respawnFromPool = 0;
    m_hasCloud = 0;
    m_animTick = 0;

    SHORT nFacing = CGameSprite::GetDirection(CPoint(0, 0), velocity);
    m_animation.m_animation = new IcewindCGameAnimationTypeEffect(descriptor, static_cast<WORD>(nFacing & 0xF));

    // 0x56DFD7 / 0x56E34F: the binary routes m_resref1 through CString(LPCSTR), which
    // substitutes "" (DAT_008485bc) for a NULL/empty slot. CResRef(const char*) is not
    // NULL-safe (it derefs pName), so build the CString first to match the binary.
    m_sound.SetResRef(CResRef(CString(descriptor.m_resref1)), TRUE, TRUE);
    m_sound.SetChannel(0xE, reinterpret_cast<DWORD>(pArea));

    m_animationRunning = 1;
    m_posExact = pos;
    m_posDelta = velocity;
    m_duration = a7;
    m_durationFade = mode8;
    m_collision = mode9;

    memcpy(m_terrainTable, CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE, sizeof(m_terrainTable));

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE)
        == CGameObjectArray::SUCCESS) {
        CPoint scaledPos(pos.x >> (CGameSprite::EXACT_SCALE & 0x1F),
            pos.y >> (CGameSprite::EXACT_SCALE & 0x1F));
        AddToArea(pArea, scaledPos, a5, CGameObject::LIST_FRONT);
    }
    // else: registration failed -> the binary self-deletes via vtable slot 0;
    // omitted here (matches the IcewindCSpellHitVisual ctor).
}

// 0x56E280. Builds one travelling particle of the detonation fan from the
// parent's m_emission2 descriptor (the 0x56DF00 sibling builds it from the
// plainer m_emission1). Loads the impact sound, seeds the particle's motion
// state from the launch parameters, registers the object and adds it to the
// area, taking a reference on the descriptor's shared cell object.
IcewindCSpellHitParticle::IcewindCSpellHitParticle(const IcewindCSpellHitEmissionRanged& descriptor,
    CGameArea* pArea, const CPoint& pos, int a5, const CPoint& velocity, SHORT a7, BYTE mode8,
    BYTE mode9)
{
    m_animTick = 0;

    // Build the per-particle detonation animation for this launch direction
    // (CGameSprite::GetDirection from the origin to the launch velocity gives the
    // 16-way facing). The embedded CGameAnimation installs its own vtable.
    SHORT nFacing = CGameSprite::GetDirection(CPoint(0, 0), velocity);
    m_animation.m_animation = new IcewindCGameAnimationTypeEffect(descriptor, static_cast<WORD>(nFacing & 0xF));

    // 0x56DFD7 / 0x56E34F: the binary routes m_resref1 through CString(LPCSTR), which
    // substitutes "" (DAT_008485bc) for a NULL/empty slot. CResRef(const char*) is not
    // NULL-safe (it derefs pName), so build the CString first to match the binary.
    m_sound.SetResRef(CResRef(CString(descriptor.m_resref1)), TRUE, TRUE);
    m_sound.SetChannel(0xE, reinterpret_cast<DWORD>(pArea));

    m_animationRunning = 1;
    m_posExact = pos;
    m_posDelta = velocity;
    m_duration = a7;
    m_durationFade = mode8;
    m_collision = mode9;

    memcpy(m_terrainTable, CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE, sizeof(m_terrainTable));

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE)
        == CGameObjectArray::SUCCESS) {
        CPoint scaledPos(pos.x >> (CGameSprite::EXACT_SCALE & 0x1F),
            pos.y >> (CGameSprite::EXACT_SCALE & 0x1F));
        AddToArea(pArea, scaledPos, a5, CGameObject::LIST_FRONT);

        // Take a counted reference on the descriptor's shared cell pool and copy
        // its trailing parameters.
        m_cellPool = descriptor.m_cellPool;
        m_cellPool->m_refCount++;
        m_cellIndex = descriptor.m_lastCellIndex;
        m_respawnFromPool = static_cast<BYTE>(descriptor.m_respawnFlag);
        m_hasCloud = static_cast<BYTE>(descriptor.m_cloudFlag);
    }
    // else: registration failed -> the binary self-deletes via vtable slot 0;
    // omitted here (matches the IcewindCSpellHitVisual ctor).
}

// 0x56E260 (vtable slot 0 -- MSVC emits the scalar-deleting wrapper there; the
// destructor body proper is 0x56E580). Releases the shared cell object the ctor
// referenced (refcount at +0x10; when it reaches zero, free its buffer and the
// object). The detonation animation, the CSound, the embedded CGameAnimation
// and the CGameObject base destruct automatically (the inlined
// CGameAnimation::~CGameAnimation is what frees the CGameAnimationTypeEffect).
IcewindCSpellHitParticle::~IcewindCSpellHitParticle()
{
    if (m_cellPool != NULL && --m_cellPool->m_refCount == 0) {
        delete m_cellPool;
    }

    // m_animation (embedded CGameAnimation) auto-destructs and deletes its own
    // m_animation pointer (CGameAnimation::~CGameAnimation, NULL-checked). The
    // binary inlines exactly that one delete; deleting it explicitly here too
    // double-frees the CGameAnimationTypeEffect -> 0xDDDDDDDD use-after-free read
    // in ~CGameAnimation when the member dtor then runs on the freed pointer.
}

// 0x56E650 (vtable slot 3). Drives the particle: drift along m_posDelta, then on
// crossing a search-grid cell bounce off / die on / pass through walls
// (CSearchBitmap::GetLOSCost), expiring after m_duration ticks or when the
// animation sequence ends, and advance the frame. Mirrors CGameTemporal::AIUpdate.
void IcewindCSpellHitParticle::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) {
        return;
    }

    m_animTick++;

    if (m_duration == 0) {
        // Lives until the animation sequence runs out.
        if (m_animation.IsEndOfSequence()) {
            RemoveFromArea();
            return;
        }
    } else {
        m_duration--;
        if (m_duration == 0) {
            RemoveFromArea();
            return;
        }
    }

    int nOldCellX = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
    int nOldCellY = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;

    m_posExact.x += m_posDelta.x;
    m_posExact.y += m_posDelta.y;
    m_pos.x = m_posExact.x >> CGameSprite::EXACT_SCALE;
    m_pos.y = m_posExact.y >> CGameSprite::EXACT_SCALE;

    int nNewCellX = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
    int nNewCellY = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;

    if (nOldCellX != nNewCellX || nOldCellY != nNewCellY) {
        SHORT nTableIndex;
        BYTE cost = m_pArea->m_search.GetLOSCost(CPoint(nNewCellX, nNewCellY),
            m_terrainTable, nTableIndex, FALSE);
        if (cost == CPathSearch::COST_IMPASSABLE) {
            if (m_collision == CGameTemporal::COLLISION_DESTROY) {
                RemoveFromArea();
                return;
            }

            if (m_collision == CGameTemporal::COLLISION_REBOUND) {
                CPoint ptOld(m_posExact);
                if (nOldCellX != nNewCellX) {
                    m_posExact.x += -m_posDelta.x * 2;
                    m_pos.x = m_posExact.x >> CGameSprite::EXACT_SCALE;
                    m_posDelta.x = -m_posDelta.x;
                }

                if (nOldCellY != nNewCellY) {
                    m_posExact.y += -m_posDelta.y * 2;
                    m_pos.y = m_posExact.y >> CGameSprite::EXACT_SCALE;
                    m_posDelta.y = -m_posDelta.y;
                }

                SHORT nDirection = CGameSprite::GetDirection(ptOld, m_posExact);
                m_animation.ChangeDirection(nDirection);
            } else {
                // Pass-through: keep drifting, hidden while inside the wall.
                m_animationRunning = 0;
            }
        } else {
            m_animationRunning = 1;
        }
    }

    // m_hasCloud (descriptor.m_cloudFlag): the ICloudA/ICloudB cloud flip-book. On
    // each end-of-sequence flip an ICloudB cell back to ICloudA, and every 100th tick
    // force it to ICloudB -- so the cloud animation alternates between the two BAMs.
    // Only the cloud AoEs set this (m_cloudFlag == 1); Fireball (m_cloudFlag == 0)
    // never enters here. OverrideAnimation lives on the CGameAnimationTypeEffect base.
    if (m_hasCloud == 1) {
        CGameAnimationTypeEffect* pAnim =
            static_cast<CGameAnimationTypeEffect*>(m_animation.m_animation);
        if (m_animation.IsEndOfSequence()) {
            if (pAnim->m_animResName.Compare(0, pAnim->m_animResName.Length(),
                    "ICloudB", static_cast<int>(strlen("ICloudB"))) == 0) {
                pAnim->OverrideAnimation("ICloudA");
            }
        }
        if (m_animTick != 0 && m_animTick % 100 == 0) {   // 0x84F260 = 100 ticks
            pAnim->OverrideAnimation("ICloudB");
        }
    }

    // m_respawnFromPool (descriptor.m_respawnFlag): on each end-of-sequence, move
    // this ember to a random unclaimed cell of the shared fan pool and release the
    // one it held, so the stationary detonation embers keep redistributing across
    // the burn footprint each cycle. Omitting it was the Fireball "green ground
    // flames" regression -- the original IWD2.exe runs this respawn; our build did
    // not, leaving the FirebaA embers stuck advancing past their sequence end.
    if (m_respawnFromPool == 1) {
        if (m_animation.IsEndOfSequence()) {
            int nIndex;
            do {
                int nCount = static_cast<int>(m_cellPool->m_cells.size());
                int nBack = 0;
                if (nCount != 0) {
                    int nRand = rand();
                    if (nRand % nCount != 0) {
                        nBack = rand() % (nRand % nCount);
                    }
                }
                nIndex = nCount - nBack - 1;
            } while (m_cellPool->m_cells[nIndex].flag == 1);

            m_cellPool->m_cells[nIndex].flag = 1;
            m_cellPool->m_cells[m_cellIndex].flag = 0;
            m_cellIndex = nIndex;
            m_posExact.x = m_cellPool->m_cells[nIndex].x;
            m_posExact.y = m_cellPool->m_cells[nIndex].y;
            m_pos.x = m_posExact.x >> CGameSprite::EXACT_SCALE;
            m_pos.y = m_posExact.y >> CGameSprite::EXACT_SCALE;
        }
    }

    m_animation.IncrementFrame();
}

// 0x56ECF0 (vtable slot 18). Removes the particle from the area and the global
// object array, then deletes itself. Mirrors CGameTemporal::RemoveFromArea with a
// trailing self-delete.
void IcewindCSpellHitParticle::RemoveFromArea()
{
    CGameObject::RemoveFromArea();

    BYTE nResult = g_pBaldurChitin->GetObjectGame()->m_cObjectArray.Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH, NULL, -1);
    if (nResult != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjAnimation.cpp __LINE__: 0x1B3
        UTIL_ASSERT(FALSE);
        return;
    }

    delete this;
}

// 0x56EA90 (vtable slot 19). Draws the particle's detonation animation through
// CInfinity, gated on m_animationRunning, the impact point being in-bounds and
// its tile being visible. Mirrors CGameTemporal::Render with an extra position
// bounds guard.
void IcewindCSpellHitParticle::Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface)
{
    (void)pArea;   // binary asserts pVidMode != NULL here (ObjAnimation.cpp:0x162); UtilAssert omitted

    if (!m_animationRunning) {
        return;
    }
    if (m_pos.x < 0 || m_pos.y < 0 || m_pos.x >= 0x1401 || m_pos.y >= 0xF01) {
        return;
    }

    CInfinity* pInfinity = &m_pArea->m_cInfinity;

    LONG nTileIndex = (m_pos.y / CVisibilityMap::SQUARE_SIZEY) * m_pArea->m_visibility.m_nWidth
        + m_pos.x / CVisibilityMap::SQUARE_SIZEX;
    if (!m_pArea->m_visibility.IsTileVisible(nTileIndex)) {
        return;
    }

    BOOL bFadeOut = FALSE;
    if (m_duration < m_durationFade) {
        bFadeOut = TRUE;
    }

    CRect rView(pInfinity->nCurrentX,
        pInfinity->nCurrentY,
        pInfinity->nCurrentX + (pInfinity->rViewPort.right - pInfinity->rViewPort.left),
        pInfinity->nCurrentY + (pInfinity->rViewPort.bottom - pInfinity->rViewPort.top));

    CRect rFx;
    CPoint ptReference;
    m_animation.CalculateFxRect(rFx, ptReference, m_posZ);

    CPoint ptNewPos(m_pos.x, m_pos.y);
    ptNewPos.y += m_pArea->GetHeightOffset(m_pos, CGameObject::LIST_BACK);

    CRect rGCBounds;
    m_animation.CalculateGCBoundsRect(rGCBounds, ptNewPos, ptReference, m_posZ,
        rFx.right - rFx.left, rFx.bottom - rFx.top);

    CRect rDest;
    if (!rDest.IntersectRect(rGCBounds, rView)) {
        return;
    }

    CPoint ptTint(m_pos.x, m_pos.y + m_posZ);
    COLORREF rgbTintColor = m_pArea->GetTintColor(ptTint, m_listType);

    m_animation.Render(pInfinity, pVidMode, nSurface, rFx, ptNewPos, ptReference,
        0x20000, rgbTintColor, rDest, FALSE, bFadeOut, m_posZ, 0);
}
