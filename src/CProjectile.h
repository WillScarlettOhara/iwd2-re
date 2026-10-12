#ifndef CPROJECTILE_H_
#define CPROJECTILE_H_

#include <list>
#include <map>
#include <set>
#include <vector>

#include "CGameAnimation.h"
#include "CGameEffectList.h"
#include "CGameObject.h"
#include "CVidCell.h"
#include "CVidPalette.h"
#include "CVidBitmap.h"
#include "CSound.h"
#include "IcewindCProjectileTargetMap.h"
#include "IcewindCVisualEffect.h"

class CGameAIBase;
class CGameAnimationTypeEffect;
class CGameArea;
class CGameSprite;
class CRes;
class CResBitmap;

#pragma pack(push, 2)
class CProjectile : public CGameObject {
public:
    static void GetStart(LONG source, CPoint& pt, BYTE facingOffset); // 0x52A640
    static CProjectile* DecodeProjectile(USHORT projectileType, CGameAIBase* pCaster, BYTE castDelay);

    CProjectile();
    /* 0000 */ ~CProjectile() override;
    /* 002C */ BOOLEAN CompressTime(DWORD deltaTime) override;

    /* 0068 */ BOOLEAN IsProjectile() override;
    /* 006C */ virtual void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType);
    /* 0070 */ virtual void OnArrival();
    /* 0074 */ virtual void RemoveSelf();
    /* 0078 */ virtual void DeliverEffects();
    /* 007C */ virtual void CallBack();

    void AddEffect(CGameEffect* pEffect);
    void ClearEffects();
    LONG DetermineHeight(CGameSprite* pSprite);
    SHORT GetDirection(CPoint target);
    BOOL IsTargetImmune(CGameSprite* pSprite);
    void PlaySound(CResRef resRef, BOOL loop, BOOL fireAndForget);

    /* 006E */ WORD m_projectileType;
    /* 0070 */ WORD field_70;
    /* 0072 */ LONG m_sourceId;
    /* 0076 */ LONG m_targetId;
    /* 007A */ LONG m_callBackProjectile;
    // The projectile's effect list is the plain MFC typed list (0x1C), NOT a
    // CGameEffectList: the sprite list's cursor tail (+0x1C..+0x2B) would
    // overlap m_sparkleColor and the travelling flight accumulators at
    // +0x9C.. (the ctor 0x530790 zeroes a WORD at +0x9A, and the factory's
    // sparkle cases stamp the colour there).
    /* 007E */ CTypedPtrList<CPtrList, CGameEffect*> m_effectList;
    // Sparkle-stream colour (1=black 2=blue 3=chromatic 4=gold 5=green
    // 6=purple 7=red 9=ice 10=stone 11=magenta 12=orange), row index into the
    // travel palette bitmap; written by the DecodeProjectile sparkle cases.
    /* 009A */ SHORT m_sparkleColor;
    // Shared launch state: both travelling and BAM Fire write these fields.
    /* 009C */ INT m_posAccumX;
    /* 00A0 */ INT m_posAccumY;
    /* 00C8 */ INT m_targetX;
    /* 00CC */ INT m_targetY;
    // BG2 PDB: CProjectile::m_terrainTable. Render's passability gate feeds
    // it to CSearchBitmap::GetMobileCost; the wandering leaves pass it to
    // GetLOSCost to bounce off walls. Seeded by the CProjectileTravelling and
    // IcewindCProjectileTravellingVFX ctors (.data 0x8A8154), not here.
    /* 00D0 */ BYTE m_terrainTable[16];
    /* 00E2 */ BOOL m_bSparkleTrail;
    /* 00EA */ CGameArea* m_pArea;
    /* 00EE */ CSound m_sound;
    /* 0152 */ CResRef m_fireSoundRef;
    /* 015A */ BOOL m_loopFireSound;
    /* 015E */ CResRef m_arrivalSoundRef;
    /* 0166 */ BOOL m_loopArrivalSound;
    /* 016A */ BOOLEAN m_bHasHeight;
    /* 016C */ SHORT m_nDeltaZ;
    /* 016E */ SHORT m_nDeltaZLast;
    /* 0170 */ LONG m_nOrigDistance;
    /* 017C */ BYTE field_17C;
    /* 017E */ CString field_17E;
    /* 0182 */ LONG m_nTargetId;
    // The level of the spell that launched this projectile -- IsTargetImmune
    // (0x536FC0) indexes the target's spell-level immunity table (the one
    // Globe of Invulnerability sets) with it. Only the AI-side
    // FireSpell/FireSpellPoint paths look up and store the real memorized
    // level; the player executors (Spell/SpellPointSequence/UseItemPoint)
    // store 0.
    /* 0186 */ LONG m_nSpellLevel;
    /* 018A */ CResRef m_casterResRef;
};
#pragma pack(pop)

// ---------------------------------------------------------------------------
// CProjectileInstant (vtable 0x84D8B8) -- the "None" projectile (missileType 1).
//
// A thin CProjectile that delivers its payload immediately: Fire records the
// source/target/area, runs OnArrival (which calls DeliverEffects) with no
// travel and no area registration, then self-deletes.  This is how an instant
// spell whose effects target the caster/preset point reaches its target
// (e.g. Call Lightning's opcode-449 Static Charge effect).  Adds no members.
// ---------------------------------------------------------------------------
class CProjectileInstant : public CProjectile {
public:
    CProjectileInstant();   // 0x529490
    void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos,
              LONG nHeight, SHORT nType) override;   // 0x529950 (slot 27)
};

class CProjectileBAM : public CProjectile {
public:
    CProjectileBAM(const CResRef& visualResRef, const CResRef& arrivalSoundRef, BYTE sequenceDelay, BYTE initialDelay, const IcewindCVisualEffect& visualEffect);

    void AIUpdate() override;
    void Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface) override;
    void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

protected:
    /* 0192 */ CVidCell m_vidCell;
    /* 026C */ CVidBitmap m_bitmap;
    /* 0326 */ BOOL field_326;
    /* 032A */ BYTE field_32A;
    /* 032B */ BYTE field_32B;
    /* 032C */ BYTE m_sequenceDelay;
    /* 032D */ BYTE m_initialDelay;
    /* 032E */ IcewindCVisualEffect m_visualEffect;
};

class CProjectileSummonVFX : public CProjectile {
public:
    CProjectileSummonVFX(const CResRef& visualResRef, const IcewindCVisualEffect& visualEffect);

    void AIUpdate() override;
    void Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface) override;
    void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;

    void SetArrivalSound(const CResRef& arrivalSoundRef);
    void SetOffsetAboveTarget(BOOL offsetAboveTarget);

    static CProjectile* DecodeSpellHitProjectile(int typeIndex, CGameAIBase* pCaster, BOOL bPositive);

private:
    /* 0192 */ CVidCell m_vidCell;
    /* 026C */ IcewindCVisualEffect m_visualEffect;
    /* 032C */ BOOL m_offsetAboveTarget;
};

// Intermediate base for the travelling weapon/spell projectiles (arrows, bolts,
// darts, axes, spears, magic missiles, fireballs). Adds a heap CVidCell
// animation cell plus a range palette and bitmap on top of CProjectile. 17 leaf
// classes derive from it (~63 DecodeProjectile cases). vtable 0x84D9C4.
//
// The core flight, render, and collision virtuals (slots 3/19/27/32/33) are
// recovered; a few leaf-specific overrides (CProjectileArrow slots 34/37) remain
// stubbed.
class CProjectileTravelling : public CProjectile {
    // The Magic Missile launcher (0x530C90) writes its sub-missiles' drift
    // fields directly (m_driftX/Y, m_driftDecay, m_hasDrift, m_velocity).
    friend class CProjectileSPMAGMIS;
    // DecodeProjectile's sparkle cases (0x2F-0x37/0x41/0xB8/0xB9) write
    // m_velocity and m_visible on the freshly built leaf.
    friend class CProjectile;
    // CProjectileCone::Pulse configures the cell/velocity/direction of the
    // per-pulse spray visuals it spawns.
    friend class CProjectileCone;

public:
    CProjectileTravelling(const CResRef& resRef);
    ~CProjectileTravelling() override;
    void AIUpdate() override;                  // vtable slot 3 (0x52B900)
    void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;  // slot 27 (0x52C050)
    void Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface) override;  // slot 19 (0x52B190)
    virtual void AimAtPoint(int x, int y);     // vtable slot 33 (0x52BD20)
    virtual DWORD GetRenderFlags();            // vtable slot 32 (0x5297D0)

    void SetVidCell(CResRef resRef);            // 0x5295D0 -- replace the animation cell
    void SetTravelPalette(CString bitmapName);  // 0x529660 -- request the sparkle colour-table bitmap (#guess name)

protected:
    void GetCellBounds(CRect& rBounds, CPoint& ptRef, CVidCell* pCell);   // 0x52B6B0 -- cell draw rect + ref point (pCell NULL = the main cell)
    void UpdateDirectionSequence(CVidCell* pCell);       // 0x52C8C0 -- pick anim sequence from facing

    CVidCell* m_pVidCell;       // +0x192 main animation cell
    CVidPalette m_palette;
    CVidBitmap m_bitmap;
    CVidCell* m_pShadowCell;    // +0x196 secondary "shadow" cell (drawn when m_hasShadowCell)
    SHORT m_velocity;           // +0x70 -- Frida-confirmed (arrival radius = velocity+1)
    int m_stepX;                // +0xA4 -- per-tick velocity step x
    int m_stepY;                // +0xA8 -- per-tick velocity step y
    int m_driftX;               // +0xAC -- per-tick lateral drift carry X (bled off by m_driftDecay)
    int m_driftY;               // +0xB0 -- per-tick lateral drift carry Y
    int m_jitterMinX;           // +0xB4 -- random step-spread band low (x)
    int m_jitterMinY;           // +0xB8 -- random step-spread band low (y)
    int m_jitterMaxX;           // +0xBC -- random step-spread band high (x)
    int m_jitterMaxY;           // +0xC0 -- random step-spread band high (y)
    int m_hasDrift;             // +0xC4 -- lateral-offset flag (1 if the launcher gave a perpendicular spread step)
    USHORT m_driftDecay;        // +0xE0 -- drift carry decay modulus per tick (launch velocity for odd-man)
    DWORD m_renderFlags;        // +0xE6 -- base blit flags (GetRenderFlags, slot 32)
    int m_flightDistSq;         // +0x170 -- flight distance^2 computed at Fire; 0 => arrived
    int m_tinted;               // +0x1BE -- Frida-confirmed (apply area tint colour)
    int m_useHeightOffset;      // +0x1C2 -- Frida-confirmed (add area height offset)
    int m_mirror;               // +0x1C6 -- Frida-confirmed (flip; adds blit flag 0x200)
    int m_mirrorMinX;           // +0x1CA -- mirror ref-point clamp X (GetCellBounds)
    int m_mirrorMinY;           // +0x1CE -- mirror ref-point clamp Y (GetCellBounds)
    SHORT m_leafRenderParam;    // +0x1D2 -- leaf-set render param (MMissiT/SPMAGMIS = 0x80)
    int m_hasShadowCell;        // +0x1D4 (param[0x75]) -- Frida-confirmed
    SHORT m_dirCount;           // +0x1D8 -- directional sequence count (16/8/1; >1 => pick by facing)
    SHORT m_direction;          // +0x1DA -- Frida-confirmed (facing; drives mirror thresholds)
    SHORT m_facing;             // +0x1DC -- movement facing (0..15, CGameSprite::GetDirection)
    int m_visible;              // +0x1DE -- Frida-confirmed (render gate)
    BOOL m_travelPaletteRequested/*#guess*/;  // +0x282 -- a CRes::Request is outstanding
    CResBitmap* m_pTravelPaletteRes/*#guess*/; // +0x286 -- the sparkle colour-table bitmap
    CResRef m_travelPaletteRef/*#guess*/;     // +0x28A -- its resref ("STTRAVL1")
    int field_298;              // +0x298 -- zeroed by SetTravelPalette
    BYTE m_paletteSwap;         // +0x29C (param[0xa7]) -- Frida-confirmed; set by SetTravelPalette, Render swaps the cell palette from the bitmap
    BYTE m_distLifetime;        // +0x29D -- gate: when set, compute lifetime from sqrt(dist)/velocity
    SHORT m_lifetime;           // +0x29E -- Frida-confirmed (decrements 1/tick from 0x7FFF)
};

// Leaf 0x5324A0 -- the canonical travelling arrow (DecodeProjectile types
// 0x2/0x5/0x6; vtable 0x84E58C). Its only own virtual is the destructor
// (0x5325E0, base dtor inlined, deferred); everything else is inherited from
// CProjectileTravelling. (The family vtables are 34 slots: 0x532860/0x5329A0,
// once mistaken for arrow slots 34/37, are slots 0/3 of the next vtable,
// CProjectileSPFLMARR's 0x84E614.)
class CProjectileArrow : public CProjectileTravelling {
public:
    CProjectileArrow();   // 0x5324A0
};

// Travelling weapon leaves; vtables 0x84E14C and 0x84E2E4 inherit all
// gameplay/render slots from CProjectileTravelling and add no members.
class CProjectileBolt : public CProjectileTravelling {
public:
    CProjectileBolt(); // 0x531360
};

class CProjectileSlingStone : public CProjectileTravelling {
public:
    CProjectileSlingStone(); // 0x5319A0
};

// Leaf 0x532720 -- the generic travelling weapon missile with a flame trail
// (vtable 0x84E614, binary sizeof 0x2A8). DecodeProjectile builds it for the
// flaming arrow (type 0x4, flying its default SPFLMARR cell), the thrown axe
// (0x9, AXE cell), crossbow bolt (0xE, BOLT), throwing dagger (0x1D, DAGGER),
// dart (0x22, DART) and the palette-tinted variants (0x66/0x67/0xBC, ranges
// 0x45/0x44/0x47). Each AI tick it drops three CGameTemporal trail puffs
// (animation set 0x301, colour ranges m_trailColorRanges) behind itself at
// 4/3, 1 and 2/3 of a velocity step. Its destructor (0x532860, scalar
// 0x532880, base dtor inlined) is the compiler-generated empty leaf part.
class CProjectileSPFLMARR : public CProjectileTravelling {
    // DecodeProjectile's tinted cases re-range the missile palette and stamp
    // the trail colour ranges on the freshly built leaf.
    friend class CProjectile;

public:
    CProjectileSPFLMARR();   // 0x532720

    void AIUpdate() override;   // 0x5329A0 (vtable slot 3)

private:
    /* 02A0 */ BYTE m_trailColorRanges[7];
    /* 02A7 */ BYTE m_trailTick;
};

// Leaf 0x531790 -- the thrown dart (DecodeProjectile types 0x20/0x23/0x24;
// vtable 0x84E25C; standard dart items carry missile type 0x24). A tinted
// 16-direction missile flying the DART cell at 3x the base velocity; its only
// own virtual is the destructor (0x531860, deferred), everything else is
// inherited from CProjectileTravelling.
class CProjectileDart : public CProjectileTravelling {
public:
    CProjectileDart();   // 0x531790
};

// Intermediate 0x52CCE0 -- the exploding weapon missile (vtable 0x84DBDC,
// binary sizeof 0x4B2). On top of the travelling flight it carries a target
// CAIObjectType, strike ranges and charges, a linger state and two extra
// animation cells, and its strike pass (AreaEffect -- the name carries over
// from the BG2 PDB's CProjectileArea) fires a child projectile (type
// m_childProjectileType + 1) at every creature in strike range, cloning this
// missile's effect list onto each child.
//
// Render 0x52CF80 (slot 19) draws the two explosion cells during the linger;
// the destructor 0x52CE10/0x52CE30 is the
// compiler-generated member/base chain. This branch's vtables are 35 slots --
// Explode is the only added virtual.
#pragma pack(push, 2)
class CProjectileExploding : public CProjectileTravelling {
    // DecodeProjectile's Skull Trap / Glyph cases (0x60/0x64) stamp the strike
    // and pre-check ranges on the freshly built leaf.
    friend class CProjectile;

public:
    CProjectileExploding(const CResRef& resRef);   // 0x52CCE0

    void Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface) override; // 0x52CF80 (slot 19)
    void AIUpdate() override;    // 0x52DD60 (slot 3)
    void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;   // 0x52D9F0 (slot 27)
    void OnArrival() override;   // 0x52D7F0 (slot 28, arms the linger state)

    // 0x78E730 (vtable slot 34; the binary impl is COMDAT-folded with the
    // no-op CProjectile::CallBack). AreaEffect fires it once any child went
    // out; the leaves override it with their explosion VFX (0x52F1C0 --
    // deferred).
    virtual void Explode();

protected:
    int AreaEffect(BYTE bCheckRange);   // 0x52D430

    /* 02A0 */ SHORT m_strikeRange;
    /* 02A2 */ SHORT m_preCheckRange;
    /* 02A4 */ SHORT m_childProjectileType;   // the strike fires type + 1
    /* 02A6 */ SHORT m_strikesLeft;
    // 0 while flying; OnArrival's linger state otherwise (AIUpdate then runs
    // the periodic strike pass instead of the flight).
    /* 02A8 */ int m_nState;
    /* 02AC */ SHORT m_lingerPeriod;
    /* 02AE */ SHORT m_lingerCountdown;
    // Nonzero: AreaEffect first verifies somebody is inside m_preCheckRange
    // and strikes nobody otherwise.
    /* 02B0 */ int m_bPreScan/*#guess*/;
    /* 02B4 */ CAIObjectType m_targetType;
    // One-shot: once one strike remains, AIUpdate fires the delayed
    // CGameFireball3d burst of m_burstType (0-5 pick the colour ranges,
    // 0xFF = none) -- the spell cases of DecodeProjectile arm these.
    /* 02F0 */ BYTE m_bBurstPending/*#guess*/;
    /* 02F1 */ BYTE m_burstType/*#guess*/;
    /* 02F2 */ int m_bCheckNonSprites;
    /* 02F6 */ int m_bExplodeCell1Active; // #guess name; first explosion cell enable
    /* 02FA */ CVidCell m_explodeCell1; // #guess name; Render/AI cell at native +0x2FA
    /* 03D4 */ int m_bExplodeCell2Active; // #guess name; second explosion cell enable
    /* 03D8 */ CVidCell m_explodeCell2; // #guess name; Render/AI cell at native +0x3D8
};

// Leaf 0x52E9F0 -- the exploding flame missile (DecodeProjectile type 0x3;
// vtable 0x84DCF4, binary sizeof 0x4BA): the SPFLMARR cell at 3x the base
// velocity with the flame-trail puffs (CGameTemporal, animation set 0x300).
// Its only own deferred virtual is the destructor 0x52EB10/0x52EB30
// (inline-base empty leaf part).
class CProjectileExplodingFlame : public CProjectileExploding {
public:
    CProjectileExplodingFlame();   // 0x52E9F0

    void AIUpdate() override;   // 0x52EC80 (vtable slot 3)
    void Explode() override;    // 0x52F1C0 (vtable slot 34)

private:
    /* 04B2 */ BYTE m_trailColorRanges[7];
    /* 04B9 */ BYTE m_trailTick;
};

// Leaf 0x52E230 -- the exploding thrown-weapon missile (DecodeProjectile
// types 0x8 AXE / 0xD BOLT / 0x12 MAGICSTN / 0x1C DAGGER / 0x21 DART /
// 0x39 SPEAR and the palette-tinted 0x68/0xCC; vtable 0x84DC68, binary
// sizeof 0x4BC): a mirrored 16-direction missile at 2x the base velocity
// flying the SPFIREBL cell until the case swaps in the weapon BAM. Drops the
// same CGameTemporal flame trail as CProjectileExplodingFlame (animation set
// 0x300) but aims only at the recorded target point (no live re-aim), and
// carries the explosion colour range the tinted cases restamp. Its only own
// deferred virtual is the destructor 0x52E360.
class CProjectileExplodingWeapon : public CProjectileExploding {
    // DecodeProjectile's tinted cases re-range the palette and stamp the
    // trail and explosion colour ranges on the freshly built leaf.
    friend class CProjectile;

public:
    CProjectileExplodingWeapon();   // 0x52E230

    void AIUpdate() override;   // 0x52E4D0 (vtable slot 3)
    void Explode() override;    // 0x52E940 (vtable slot 34)

private:
    /* 04B2 */ BYTE m_trailColorRanges[7];
    /* 04B9 */ BYTE m_trailTick;
    /* 04BA */ BYTE m_explodeColorRange/*#guess*/;
};

// Leaf 0x52F260 -- Skull Trap (DecodeProjectile type 0x60; vtable 0x84DD80,
// binary sizeof 0x4B8). An exploding spell projectile: it flies its missile
// cell and, on arrival, lingers and strikes via CProjectileExploding. Beyond
// the base it builds a second (shadow) animation cell from the explosion BAM
// and arms both embedded explosion cells (m_explodeCell1/m_explodeCell2) with
// the missile and explosion resrefs, and flies at double the launch velocity.
// Its AIUpdate (0x52F9E0, slot 3) and Explode (slot 34) overrides are deferred;
// until they are recovered it ticks with the base exploding behaviour. The
// compiler-generated destructor matches the binary's 0x52F5E0 (it adds only the
// +0x4B4 CString to the base members).
class CProjectileSkullTrap : public CProjectileExploding {
    // DecodeProjectile (types 0x60/0x64) stamps the explosion sound.
    friend class CProjectile;

public:
    CProjectileSkullTrap(const CResRef& cMissileRef, const CResRef& cExplodeRef, SHORT nType);   // 0x52F260

    void AIUpdate() override;   // 0x52F9E0 (vtable slot 3)
    void Explode() override;    // 0x52F760 (vtable slot 34)

private:
    /* 04B2 */ SHORT m_nType;
    /* 04B4 */ CString m_explodeSound;   // explosion sound resref; default-empty (ctor seeds the nil sentinel)
};
#pragma pack(pop)

// Leaf 0x5300E0 -- the strike bolt (DecodeProjectile type 0x4F; vtable
// 0x84DF1C): the invisible effect carrier the exploding missiles' strike
// pass (CProjectileExploding::AreaEffect) fires at each target. The SPFIREBL
// cell never draws (m_visible = 0) and the flight lifetime derives from the
// launch distance (m_distLifetime). Its only own virtual is the destructor
// (0x5301C0, inline-base empty leaf part, deferred).
class CProjectileStrike : public CProjectileTravelling {
public:
    CProjectileStrike();   // 0x5300E0
};

// Leaf 0x57E030 -- the Magic Missile homing sub-missile (DecodeProjectile type
// 0xDA; vtable 0x8510A4). Spawned in quantity by the Magic Missile launcher
// (CProjectileMagicMissileMulti), not cast directly. A directional (mirrored)
// travelling missile at double the base velocity.
class CProjectileMMissiT : public CProjectileTravelling {
public:
    CProjectileMMissiT(SHORT nPaletteFlag);   // 0x57E030
};

// Leaf 0x52CA10 -- the single travelling spell missile ("SPMAGMIS" BAM,
// vtable 0x84DB54): the plain Magic Missile bolt (DecodeProjectile type 0x25)
// and, with the cell swapped to "TRAVEL" and a colour stamped, the coloured
// sparkle streams (types 0x2F-0x37/0xB8/0xB9) plus the invisible gaze carrier
// (0x41). A mirrored missile at double the base velocity, like CProjectileMMissiT,
// with the directional fields explicitly cleared. Its only own virtual is the
// destructor (0x52CBC0 -- empty leaf part, base dtor inlined, deferred); it
// flies, renders and delivers effects through the CProjectileTravelling/
// CProjectile base path. (0x52CE10/0x52DD60, once mistaken for sparkle slots
// 34/37, are slots 0/3 of the next vtable, CProjectileExploding's 0x84DBDC.)
class CProjectileSparkle : public CProjectileTravelling {
public:
    CProjectileSparkle(SHORT nPaletteType);   // 0x52CA10
};

// Intermediate base for the Magic Missile launcher (ctor 0x5309C0). Pre-spawns a
// list of sub-missiles in its constructor; the launcher's Fire drains the list
// into the area. The original multiply-inherits the sub-missile list at +0x2A2
// (a CPtrList, secondary vtable 0x84E0B8); we model it as a member -- it is
// transient launch staging, so the layout difference is immaterial.
class CProjectileMagicMissileMulti : public CProjectileTravelling {
public:
    CProjectileMagicMissileMulti(const CResRef& resRef, SHORT nCount, USHORT nSubType, BYTE nPaletteFlag);

protected:
    SHORT m_missileCount;                                  // +0x2A0
    CTypedPtrList<CPtrList, CProjectile*> m_subMissiles;   // +0x2A2 (binary: a 2nd base)
    USHORT m_subType;                                      // +0x2BE
};

// Leaf 0x531120 -- the Magic Missile launcher (DecodeProjectile types 0x44-0x48,
// one per caster-level band = 1..5 missiles). Spawns N MMissiT sub-missiles in
// its base, then fires them. vtable 0x84E0C4.
class CProjectileSPMAGMIS : public CProjectileMagicMissileMulti {
public:
    CProjectileSPMAGMIS(SHORT nCount, SHORT nPaletteFlag);   // 0x531120
    void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;  // 0x530C90

private:
    // Per sub-missile prep inlined twice inside Fire (0x530C90).
    void PrimeAndFireSubMissile(CProjectileTravelling* pMissile, CGameArea* pArea,
        LONG source, LONG target, CPoint targetPos, SHORT nType);
};

// Intermediate base of the IWD2-only spell projectiles that fly a heap BAM
// cell with an attached IcewindCVisualEffect. DecodeProjectile builds it
// directly for the travelling spell bolts (types 0x18/0xFB IcelanT/0x10C
// DisintT/0x10F OFSpheT/0x11D MSporeT/0x12A ALanceT/0x13C/0x158 MFMissT), and
// its ctor (0x578110, 21 callers) is chained by CProjectileSummonVFX
// (0x57E490), the 68-case spell-hit class (0x56EDD0), the wandering family
// (Whirlwind 0x57F640, 0x57D390, 0x57F390, 0x5806C0, 0x580C00, WoMoonX
// 0x5802B0) and 0x57AEB0/0x57C510/0x57E370/0x581060/0x581CA0. vtable
// 0x850CAC; binary sizeof 0x2AE.
//
// In the binary it derives CProjectile directly and duplicates
// CProjectileTravelling's layout verbatim (same offsets: heap cell +0x192,
// shadow cell, palette +0x19A, bitmap +0x1E2, motion fields, lifetime trio
// +0x29C); we derive CProjectileTravelling instead so those members and the
// recovered flight virtuals are shared (same modelling licence as
// CProjectileMagicMissileMulti). Differences from the travelling ctor, per
// the 0x578110 asm: height handling on by default (m_bHasHeight,
// m_useHeightOffset), the four +0xD0 colors seeded, the +0x2A0/+0x2A1 flags,
// and the visual-effect member. The already-recovered chainers
// (CProjectileSummonVFX) keep their independent minimal models for now.
#pragma pack(push, 2)
class IcewindCProjectileTravellingVFX : public CProjectileTravelling {
    // The strike tracker's gather pass reads m_terrainTable off its owner.
    friend class IcewindCProjectileTargetMap;
    // DecodeProjectile's CHROMORB case (0x18) arms copy-from-back on the
    // freshly built projectile's visual effect.
    friend class CProjectile;
    // CProjectileCone::Pulse copies its visual effect onto the spray visuals.
    friend class CProjectileCone;

public:
    IcewindCProjectileTravellingVFX(const CResRef& resRef);   // 0x578110

    void AIUpdate() override;                 // 0x578AB0 (vtable slot 3)
    void Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface) override;   // 0x578480 (slot 19)
    void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;   // 0x5791D0 (slot 27)
    void AimAtPoint(int x, int y) override;   // 0x578ED0 (slot 33)

protected:
    void GetCellBounds(CRect& rBounds, CPoint& ptRef, CVidCell* pCell);   // 0x578970 -- union of the cell (z-lifted) and shadow-cell bounds
    void UpdateDirectionSequence(CVidCell* pCell);   // 0x579860 -- facing -> sequence, folding mirrored facings per the flags below

    // The BAM has no north/east-half sequences: fold those facings onto the
    // south/west ones and blit with CInfinity::MIRROR_FX.
    /* 02A0 */ BYTE m_bMirrorNorth; // #guess
    /* 02A1 */ BYTE m_bMirrorEast; // #guess
    /* 02A2 */ IcewindCVisualEffect m_visualEffect;
};
#pragma pack(pop)

// Intermediate base 0x56EDD0 -- the large spell-hit / area-of-effect projectile
// (vtable 0x84F1C4; : IcewindCProjectileTravellingVFX). Unnamed in the binary
// (no BG2/RTTI carry-over); the AOE spell-effect family builds through it --
// Fireball (0x571E80, vtable 0x84F580) and ~110 other derived ctors stamp their
// own state on top. The ctor seeds the invisible "SPMAGMIS" carrier, builds the
// target filter (CAIObjectType::ANYONE), two spare animation cells, three
// visual-emission slots (each two refcounted resource names plus an
// IcewindCVisualEffect) and two sounds, then broadcasts the projectile type byte
// across the per-slot flag fields.
//
// Two embedded associative containers (VC6 _Tree, 16 bytes each in the binary):
//   m_miniA (+0x4C4, ctor 0x570D50, shared nil DAT_008e3e38)  = std::map<LONG, int>
//     -- per-target re-strike interval clock (GatherTargets, 0x56FED0): each
//        scanned victim's consecutive in-range pass count, due a strike whenever
//        the count is a multiple of m_strikeInterval. Typed as std::map plus an 8-byte
//        pad: our VS2019 _Tree is 8B on Win32 (not the binary's 16B VC6 _Tree),
//        and the pad keeps every field below it at its binary offset.
//   m_miniB (+0x64E, ctor 0x4C4A90, shared nil DAT_008d48b4) = std::set<LONG>
//     -- already-struck-target dedup set (declared below; it is the final member
//        so the by-name size drift touches nothing else).
// The base ctor stamps the two _Alval padding bytes (= type byte, don't-care) and
// the _Multi flag (= 0) of each; the container ctors fill _Myhead / _Mysize.
class IcewindCProjectileSpellHit /*#guess*/ : public IcewindCProjectileTravellingVFX {
    // DecodeProjectile (a CProjectile method) builds the bare spell-hit carrier
    // for the leaf-less area spells (case 0xFF, projectile 255) and stamps its
    // emission slot-0 detonation visual directly -- the same member-poke licence
    // the family base already grants CProjectile (IcewindCProjectileTravellingVFX).
    friend class CProjectile;

public:
    IcewindCProjectileSpellHit(SHORT nType);   // 0x56EDD0
    ~IcewindCProjectileSpellHit() override;     // 0x56F1F0 (deleting thunk 0x56F070, vtable slot 0)

    // Per-tick update: fly to the target (snap or home), then on detonation run
    // a strike pass every m_strikePeriod ticks (the m_strikeCountdown clock) until
    // m_lifetime expires. Frozen by Time Stop unless this is the time-stop caster's
    // own projectile.
    void AIUpdate() override;                                                   // 0x56FAF0 (slot 3)

    // Arrival: hand off to the call-back projectile (CallBack), flip into the
    // detonation state (m_bDetonated = 1) so AIUpdate begins strike passes, play the
    // arrival sound, then spawn the detonation FX -- the shared cell pool, the
    // IcewindCSpellHitVisual on-ground visual and the impact/loop sounds.
    void OnArrival() override;                                                  // 0x56F410 (slot 28)

    // Launch: record source/target/area, share the caster to read the launch
    // origin and height, register in the object array and area, seed the subpixel
    // flight toward the target, and pull the lifetime from the trailing effect.
    void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;  // 0x56F820 (slot 27)

    // Pure forwarder to the family Render (0x578480); the leaf still owns vtable
    // slot 19 so the slot pointer matches the binary (0x56F3F0, not the inherited
    // 0x578480). No added behaviour.
    void Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface) override;   // 0x56F3F0 (slot 19)

    // First of the five spell-hit virtuals. A no-op in every family vtable (the
    // COMDAT-folded empty 0x78E730), never overridden by a leaf and never called
    // on the recovered Fire/AIUpdate/OnArrival/strike path -- declared only so the
    // slots below land at their binary indices. Name guessed from the parallel
    // slot-34 detonation hook on the CProjectileExploding branch.
    virtual void Explode() /*#guess*/;                               // 0x78E730 (vtable slot 34, folded no-op)

    // Lifetime for a freshly fired projectile, given the trailing effect's
    // first-call byte. The base just echoes the m_lifetime default (0x5703E0 is a
    // one-line getter); a subclass overrides it to derive a duration.
    virtual LONG DetermineLifetime(BYTE bFirstCall) /*#guess*/;       // 0x5703E0 (vtable slot 35)

    // Gather every m_targetType object within range (front and back area lists)
    // and return the ids due a strike this pass: each victim is tracked in
    // m_miniA on first contact and is due whenever its in-range pass count is a
    // multiple of m_strikeInterval, while tracked victims that left the radius are
    // dropped. The family's fused counterpart to IcewindCProjectileTargetMap's
    // split GatherTargets + CollectDueStrikes.
    virtual std::list<LONG> GatherTargets();         // 0x56FED0 (vtable slot 36)

    // Strike every victim in the gathered list, delivering one strike to each
    // (the family's counterpart to IcewindCProjectileTargetMap::Strike). New
    // virtuals the spell-hit family introduces on top of the wandering-VFX base.
    virtual void Strike(std::list<LONG>& targets);   // 0x5701B0 (vtable slot 37)
    virtual void StrikeTarget(LONG targetId);        // 0x5701E0 (vtable slot 38)

    // 0x5704E0 (non-virtual).  Seed the strike target filter from a source's
    // allegiance so the bolts hit that source's enemies.  Used by the Call
    // Lightning / Static Charge engine when arming each strike bolt.
    void SetStrikeTargetFilter(CGameObject* pSource);

    // 0x5703F0 (non-virtual).  The mirror of SetStrikeTargetFilter: the same
    // allegiance test, the two EnemyAlly constants swapped, so the strike pass
    // gathers the source's OWN side.  DecodeProjectile arms it on the beneficial
    // area spells (Bless, Defensive Harmony, Mass Invisibility, Invisibility
    // Sphere, Mass Haste, ...).  Name guessed -- the class carries no RTTI or
    // BG2 PDB carry-over.
    void SetStrikeAllyFilter(CGameObject* pSource);   // #guess (name only)

protected:
    // The IE reference-counted string CProjectileCone inlines for its cone-BAM
    // name (share-count byte at block[-1], character data at block + 1); here it
    // recurs six times, each preceded by a flag byte the ctor stamps with the
    // projectile type. Cleared to empty (NULL pointer) by the ctor.
    struct ResName /*#guess*/ {
        // Copy `name` into the reference-counted buffer (releasing any current
        // block first), the way CProjectileCone inlines its cone-name copy. The
        // derived ctors reach this through the shared IE-string assign 0x537220
        // (the compiler inlines some of the calls).
        void Set(const char* name);
        // Drop this slot's share of the reference-counted block (binary: the
        // shared IE-string clear 0x448D50 with release; the dtor inlines half of
        // the six). Count 0 (sole owner) or the 0xFF sentinel frees it, otherwise
        // the count is decremented. The owning projectile's dtor calls this for
        // each slot (the six names are not auto-released).
        void Release();

        /* 00 */ BYTE  m_flags;     // ctor: = (BYTE)nType
        /* 01 */ BYTE  _pad[3];
        /* 04 */ char* m_pName;     // refcounted block + 1 (NULL when empty)
        /* 08 */ LONG  m_nameLen;
        /* 0C */ LONG  m_nameCap;
    };
    // One detonation emission slot: the visual cell BAM, a sound resref and a
    // visual-effect block. OnArrival reinterpret_casts the three slots to the
    // IcewindCSpellHitEmission(Ranged) descriptors: m_cellResRef == emission
    // m_resref0 (the BAM drawn by the spell-hit visual), m_soundResRef == emission
    // m_resref1 (played as the impact one-shot for slot 0 / looping ambience for
    // slot 2; unused for slot 1).
    struct VisualSlot /*#guess*/ {
        /* 00 */ ResName              m_cellResRef;    // emission m_resref0 -- detonation BAM cell
        /* 10 */ ResName              m_soundResRef;   // emission m_resref1 -- impact/loop sound
        /* 20 */ IcewindCVisualEffect m_fx;
    };

    // pack(2): IWD2.exe packs the whole projectile to 2 bytes -- the binary-mirror
    // members below hold many sub-4-aligned LONG/CSound/pointer tails (field_2FA,
    // m_visual1, m_visual2MaxSpawn, m_sound1, ...). Without it the compiler 4-aligns
    // each, drifting it and every member below; the visible failure was the Cloudkill
    // ring over-density (m_visual2's reinterpret_cast<IcewindCSpellHitEmission&> read a
    // 4-aligned m_maxMovingSpawn as garbage). Packs member PLACEMENT only; the nested
    // ResName/VisualSlot above keep their own default layout, as before.
#pragma pack(push, 2)
    /* 02AE */ SHORT         m_aoeRange;     // gather/detonation radius (GatherTargets `range`, visual ctor `nRange`) -- NOT the factory type
    /* 02B0 */ WORD          field_2B0;
    /* 02B2 */ WORD          m_objectTag;    // = 0x4E
    /* 02B4 */ WORD          field_2B4;
    /* 02B6 */ LONG          m_bDetonated;   // AIUpdate phase: 0 = in flight, 1 = detonating (OnArrival flips to 1)
    /* 02BA */ WORD          field_2BA;
    /* 02BC */ WORD          field_2BC;
    /* 02BE */ CAIObjectType m_targetType;   // CGameObject default = NOT_SPRITE (@0x8C76C8)
    /* 02FA */ LONG          field_2FA;
    /* 02FE */ BYTE          field_2FE;
    /* 02FF */ BYTE          field_2FF;
    /* 0300 */ LONG          m_bAffectNonCreatures;  // GatherTargets `checkForNonSprites`; StrikeTarget also hits non-creatures when set
    /* 0304 */ LONG          m_bAnimateCell1;        // AIUpdate gates m_cell1.FrameAdvance
    /* 0308 */ CVidCell      m_cell1;
    /* 03E2 */ LONG          m_bAnimateCell2;        // AIUpdate gates m_cell2.FrameAdvance
    /* 03E6 */ CVidCell      m_cell2;
    /* 04C0 */ LONG          m_lifetime;     // = 0x2D; AIUpdate decrements each tick, RemoveSelf at < 1; DetermineLifetime returns it
    // m_miniA -- per-target re-strike clock (GatherTargets, 0x56FED0). The binary
    // VC6 _Tree is 16 bytes (_Alval bytes +0x4C4/+0x4C5, _Myhead +0x4C8, _Multi
    // +0x4CC, _Mysize +0x4D0); our VS2019 std::map is 8 bytes on Win32, so the
    // 8-byte pad preserves the binary's 16-byte footprint and keeps m_strikePeriod and
    // everything below at its binary offset. Node layout drifts by name (accepted,
    // like m_miniB).
    /* 04C4 */ std::map<LONG, int> m_miniA;
    /* 04CC */ BYTE          _miniA_pad[8];
    /* 04D4 */ LONG          m_strikePeriod;     // = 10000; ticks between strike passes (reload for m_strikeCountdown)
    /* 04D8 */ LONG          m_strikeCountdown;  // = 0; AIUpdate: --; at < 1 runs GatherTargets+Strike, then reloads from m_strikePeriod
    /* 04DC */ LONG          m_strikeInterval;   // = 10; re-strike cadence: a target is due when its in-range pass count % this == 0
    /* 04E0 */ BYTE          m_bHasTravelCell;   // = 0; 0 = arrive instantly (Fire launches at targetPos), 1 = fly the travel cell
    /* 04E1 */ BYTE          _pad4E1;
    /* 04E2 */ VisualSlot    m_visual1;
    // m_visual2's IcewindCSpellHitEmission tail (the bytes past VisualSlot's 0x2C
    // {cell,sound,fx} prefix): the slot's m_animMode and m_maxMovingSpawn. Modelled
    // as named members because VisualSlot covers only the prefix. The class-level
    // pack(2) keeps m_visual2MaxSpawn at the packed m_visual2+0x2E (not the 4-aligned
    // +0x30) so OnArrival's reinterpret_cast reads the real m_maxMovingSpawn -- the
    // Cloudkill ring otherwise runs uncapped (36 emit vs the intended 13).
    /* 050E */ VisualSlot    m_visual2;
    /* 053A */ BYTE          m_visual2AnimMode;  // = 0; m_visual2 emission m_animMode (+0x2C)
    /* 053B */ BYTE          _pad53B;
    /* 053C */ LONG          m_visual2MaxSpawn;  // = 0x7FFFFFFF; m_visual2 emission m_maxMovingSpawn (+0x2E)
    // m_visual3's IcewindCSpellHitEmissionRanged tail (the block past the VisualSlot
    // prefix): the shared fan-cell pool and the spawn-density/timing fields the
    // spell-hit visual and its particles read through the emission2 reference.
    // Defaults 250/6/30 come from the 0x56FE30 emission ctor; leaf ctors override
    // these through the projectile-relative offsets.
    /* 0540 */ VisualSlot    m_visual3;
    /* 056C */ LONG          m_visual3CellPool;       // = 0; emission m_cellPool (+0x2C)
    /* 0570 */ LONG          m_visual3LastCellIndex;  // = 0; emission m_lastCellIndex (+0x30)
    /* 0574 */ BYTE          m_visual3RespawnFlag;    // = 0; emission m_respawnFlag (+0x34) -> particle m_respawnFromPool
    /* 0575 */ BYTE          m_visual3AnimMode;       // = 0; emission m_animMode (+0x35)
    /* 0576 */ BYTE          m_visual3AnimFlag36;     // = 0; emission m_animFlag36 (+0x36)
    /* 0577 */ BYTE          _pad577;
    /* 0578 */ LONG          m_visual3DensityBase;    // = 0xFA (250); emission m_densityBase (+0x38)
    /* 057C */ LONG          m_visual3EmitPeriod;     // = 6; emission m_emitPeriod (+0x3C)
    /* 0580 */ LONG          m_visual3DensityRampDiv; // = 0x1E (30); emission m_densityRampDiv (+0x40)
    /* 0584 */ BYTE          m_visual3CloudFlag;      // = 0; emission m_cloudFlag (+0x44)
    /* 0585 */ BYTE          _pad585;
    /* 0586 */ CSound        m_sound1;
    /* 05EA */ CSound        m_sound2;
    // m_miniB -- already-struck-target dedup set (binary: std::set<LONG>, ctor
    // 0x4C4A90 via shared nil DAT_008d48b4, Find 0x570620, teardown 0x5370C0).
    // A strike pass looks up each gathered target here before delivering, so a
    // target is hit at most once per pass; stays empty for one-shot AoEs
    // (fireball -- every target struck once). Final member, so the VS2019-vs-VC6
    // _Tree size drift (8/12B vs 16B) moves nothing else.
    /* 064E */ std::set<LONG> m_miniB;
#pragma pack(pop)
};

// Leaf 0x57F640 -- the wandering tornado (WhirlwX BAM; DecodeProjectile
// type 0x131, m_projectileType 0x130). Used by Whirlwind (SPPR613) and Wing
// Buffet (SPIN159); Wall of Moonlight (WoMoonX, ctor 0x5802B0, factory type
// 0x130) is the sibling family. vtable 0x851444 (34 slots; Render and
// GetRenderFlags are inherited -- slot 19 = 0x578480, the family
// IcewindCProjectileTravellingVFX::Render). Wanders the area
// from its wander seed (MP-replicated through CMessageFireProjectile +0x20)
// and strikes everything it touches through the embedded
// IcewindCProjectileTargetMap (period 3, re-strike interval 33, max 8 total
// strikes, spares the caster, gather radius 70).
class CProjectileWhirlwind : public IcewindCProjectileTravellingVFX {
public:
    CProjectileWhirlwind();        // 0x57F640
    ~CProjectileWhirlwind() override;   // 0x57F760 (vtable slot 0)

    void AIUpdate() override;      // 0x57F8D0 (slot 3)
    void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;   // 0x57FF80 (slot 27)
    void OnArrival() override;     // 0x580270 (slot 28)

    POINT* PickWanderPoint(POINT* pResult, BOOL bReverseFacing);   // 0x5800E0

    /* 02AE */ LONG m_nLifetime;
    /* 02B2 */ LONG m_nLegBudget;
    /* 02B6 */ LONG field_2B6;
    /* 02BA */ IcewindCProjectileTargetMap m_targetMap;
    /* 02F0 */ CSound m_loopSound;
    /* 0354 */ BYTE m_bFinishing;
    /* 0356 */ LONG m_wanderSeed;
};

// vtable 0x85101C, sizeof 0x2EC. The older, wall-rebounding LightnT carrier.
// The exported CProjectileBAM::MoveUpdate name at 0x57D9F0 is stale: slot 3
// of this leaf is AIUpdate, and +0x2B6 is a target scheduler, not a vid cell.
#pragma pack(push, 2)
class CProjectileReboundingBolt /*#guess*/ : public IcewindCProjectileTravellingVFX {
public:
    CProjectileReboundingBolt(); // 0x57D390
    void AIUpdate() override; // 0x57D9F0
    void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos,
        LONG nHeight, SHORT nType) override; // 0x57D700

    /* 02AE */ LONG m_nDuration;
    /* 02B2 */ LONG m_nBounces;
    /* 02B6 */ IcewindCProjectileTargetMap m_targetMap;
};
#pragma pack(pop)

// Leaf 0x5806C0 -- the straight-line beam that strikes everything along its
// path (vtable 0x8513BC; ctor takes the travelling BAM). DecodeProjectile
// builds it for Lightning Bolt (type 0x28, "LightnT"; SPWI002/308/997 + Eye of
// the Mage), Smashing Wave (0x12E, "SWaveX"; SPPR522), Lance of Disruption
// (0x139, "LoDisrT"; SPWI319) and the acid breath weapon (0x17A, "HDABreT";
// SPIN222) -- the per-case fields differ but the class is one.
//
// BG2's CProjectileLightningBolt is a CProjectileBAM with a DeliverEffects
// override; IWD2 reworked it into this Icewind travelling-VFX leaf that drives
// an embedded IcewindCProjectileTargetMap (service period 2, re-strike interval
// 33, gather range set per case) the same way CProjectileWhirlwind does. Fire
// re-aims the shot to m_beamRange past the picked target along the caster->
// target line, so the bolt overshoots and rakes the whole line.
class CProjectileLightningBolt : public IcewindCProjectileTravellingVFX {
public:
    CProjectileLightningBolt(const CResRef& resRef);   // 0x5806C0
    ~CProjectileLightningBolt() override;              // 0x580770 (vtable slot 0)

    void AIUpdate() override;      // 0x5808A0 (slot 3)
    void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;   // 0x580B30 (slot 27)

    // Distance the shot is pushed past the picked target (ScaleToCircle radius
    // in Fire); the rake length, not a lifetime. 400 or 600 per spell.
    /* 02AE */ LONG m_beamRange;
    /* 02B2 */ IcewindCProjectileTargetMap m_targetMap;
};

// Leaf 0x579B40 -- the cone/spray area effect (vtable 0x850DDC). A single
// generic class for the whole cone family: DecodeProjectile builds it for
// Burning Hands, Cone of Cold, Color Spray, Prismatic Spray, the Shout/Great
// Shout sonic cones, Frost Fingers, the Will-o-Wisp spray and the dragon/breath
// cones (14 ability-header cases, each passing an empty carrier-cell name and
// the cone's own BAM as the second argument).
//
// BG2 split this into CProjectileConeOfCold and CProjectileColorSpray (both
// CProjectileBAM leaves, sizeof 0x2A0, with static CLOCK*/ANTICLOCK* edge
// tables and a DoLayers per-layer emit); IWD2 unified them into this single
// IcewindCProjectileTravellingVFX leaf. It carries an invisible MMissiT cell
// and paints the cone by pulsing a new virtual (slot 34, the BG2 DoLayers)
// every m_pulsePeriod ticks until m_duration expires, building the cone
// edge-point list in Fire from the caster position.
//
// The binary constructs the base subobject through a cone-specific, cell-less
// emission of the IcewindCProjectileTravellingVFX ctor (0x577E40); per the
// family modelling licence that is represented here as the ordinary base ctor
// plus the carrier-cell swap in the body (the throwaway base cell the binary
// elides).
class CProjectileCone : public IcewindCProjectileTravellingVFX {
public:
    CProjectileCone(const CResRef& cellBam, const CResRef& coneBam);   // 0x579B40
    ~CProjectileCone() override;              // 0x579DE0 (slot 0; deleting thunk 0x579DC0)

    void AIUpdate() override;                 // 0x579E50 (slot 3)
    void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType) override;   // 0x579EF0 (slot 27)
    void OnArrival() override;                // 0x57A530 (slot 28)
    void DeliverEffects() override;           // 0x57A670 (slot 30)

    // New virtual (vtable slot 34, the BG2 CProjectileConeOfCold::DoLayers):
    // emits one cone layer/pulse. AIUpdate calls it every m_pulsePeriod ticks.
    virtual void Pulse();                     // 0x57A970

    // -- cone geometry, built in Fire from the caster position --
    /* 02AE */ CPoint m_edge0;                // four cone corner points (Fire trig)
    /* 02B6 */ CPoint m_edge1;
    /* 02BE */ CPoint m_edge2;
    /* 02C6 */ CPoint m_edge3;
    /* 02CE */ LONG m_coneLength;             // ctor 200
    /* 02D2 */ LONG m_outerRadius/*#guess*/;  // ctor 0x15E (350); Fire trig radius
    /* 02D6 */ LONG m_segCount;               // Fire: edge segment count
    /* 02DA */ LONG m_coneRadius/*#guess*/;   // ctor 0x19 (25)
    /* 02DE */ BYTE field_2DE;                // ctor = coneBam[0]
    /* 02DF */ BYTE _pad2DF[3];
    /* 02E2 */ char* m_pName;                 // refcounted cone-BAM name buffer
    /* 02E6 */ LONG m_nameLen;
    /* 02EA */ LONG m_nameCap;
    /* 02EE */ LONG field_2EE;                // ctor 1
    // The cone fan: Fire fills it with the per-segment arc points. A 16-byte
    // std::vector (this build's debug layout: proxy + first/last/end at +0x2F6/
    // FA/FE), matching the binary's std::vector<CPoint>.
    /* 02F2 */ std::vector<CPoint> m_edgePoints;
    /* 0302 */ LONG m_segmentStep/*#guess*/;  // ctor 0x14 (20); Fire divides m_outerRadius by it
    /* 0306 */ LONG field_306;                // ctor 0x23 (35)
    /* 030A */ LONG field_30A;                // ctor 0x2D (45)
    /* 030E */ LONG m_duration;               // ctor 0xF (15); AIUpdate finish threshold
    /* 0312 */ LONG m_pulsePeriod;            // ctor 5; AIUpdate pulse interval
    /* 0316 */ BYTE field_316;                // ctor 0
    /* 0317 */ BYTE field_317;                // ctor 1
    /* 0318 */ CPoint m_casterPos;            // Fire
    /* 0320 */ CPoint m_center;               // Fire
    /* 0328 */ LONG m_nHeight;                // Fire
    /* 032C */ SHORT m_nType;                 // Fire
    /* 032E */ LONG m_tickCount;              // ctor 0; AIUpdate counter
    /* 0332 */ BYTE m_bFinishing;             // ctor 0
};

// Leaf (vtable 0x850DDC's sibling 0x850D54) -- the per-pulse cone "spray"
// visual that CProjectileCone::Pulse fires along each fan point. A trivial
// IcewindCProjectileTravellingVFX with no own members (it flies the cone BAM
// and inherits the family flight/render); only its destructor is its own.
class CProjectileConePulseVisual : public IcewindCProjectileTravellingVFX {
public:
    CProjectileConePulseVisual(const CResRef& resRef);   // inlined into CProjectileCone::Pulse
    ~CProjectileConePulseVisual() override;              // 0x579B10 (vtable slot 0)
};

// Leaf 0x571E80 -- Fireball (SPWI304). The first of the IcewindCProjectileSpellHit
// derived AOE leaves (own vtable 0x84F580). Adds no data of its own: the ctor
// just re-points the vtable and configures the inherited spell-hit state -- flies
// the "FirebaT" travel cell, plays "TRA_06", loads the explosion/range visuals
// ("FirebaX"/"RNG_M03"/"FirebaR"/"FirebaA") into the three emission slots with
// copy-from-back enabled, doubles the launch velocity and sets m_aoeRange 200.
class CProjectileFireball /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileFireball();    // 0x571E80
    ~CProjectileFireball() override;   // 0x5768A0 (deleting thunk, slot 0; adds no data, chains to base)
};

// Leaf 0x574B80 -- Stinking Cloud (SPWI213, factory type 95/0x5F). A sibling of
// CProjectileFireball: another bare IcewindCProjectileSpellHit AOE leaf whose own
// vtable (0x8501B0) is byte-identical to Fireball's 40 slots -- it overrides nothing,
// so the two classes differ only by RTTI and ctor. Unlike Fireball it builds no travel
// cell (invisible in flight) and plays no fire sound; the ctor loads the cloud burst /
// range visuals ("SCloudX"/"RNG_M01" + "SCloudR" + "SCloudA") into the three emission
// slots with copy-from-back, plus the persistent gas area resref "ARE_M02" in the third
// slot's second name. Lifetime (m_lifetime) 1000, m_dirCount 1, m_aoeRange 100.
class CProjectileStinkingCloud /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileStinkingCloud();    // 0x574B80
    ~CProjectileStinkingCloud() override;   // slot 0; ICF-folded onto 0x5768A0 (shared with CProjectileFireball::~CProjectileFireball)
};

// Leaf 0x574EB0 -- Web (SPWI215, factory type 64/0x40). Another bare
// IcewindCProjectileSpellHit AOE leaf (own vtable 0x8502E8; dtor ICF-folded onto
// 0x5768A0). Invisible in flight, no fire sound; the ctor loads the web burst into
// the first emission slot ("WebX"/"EFF_M19") and the persistent web area into the
// third ("WebA"/"ARE_M03"), both copy-from-back. Lifetime (m_lifetime) 0x5DC, m_aoeRange
// 0x96. Leaves m_visual2 empty.
class CProjectileWeb /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileWeb();    // 0x574EB0
    ~CProjectileWeb() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x573460 -- Ice Storm (SPWI404, factory type 98/0x62). Bare
// IcewindCProjectileSpellHit AOE leaf (own vtable 0x84FAFC; dtor ICF-folded onto
// 0x5768A0). The ctor loads the storm burst into the first slot ("IStormX",
// copy-from-back) and the persistent ice area into the third ("IStormA"/"ARE_M04").
// Re-strike clock m_strikeInterval 10000, lifetime (m_lifetime) 100, m_dirCount 1, m_aoeRange
// 200. Leaves m_visual2 (and m_visual1.m_soundResRef) empty.
class CProjectileIceStorm /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileIceStorm();    // 0x573460
    ~CProjectileIceStorm() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x571CE0 -- Entangle (SPPR105, factory type 235/0xEB). Bare
// IcewindCProjectileSpellHit AOE leaf (own vtable 0x84F4E4; dtor ICF-folded onto
// 0x5768A0). The ctor loads the entangle burst into the first slot ("EntangX",
// copy-from-back) and the persistent entangling area into the third
// ("EntangA"/"ARE_P01"). Lifetime (m_lifetime) 1000, m_aoeRange 200. Leaves m_visual2
// (and m_visual1.m_soundResRef) empty.
class CProjectileEntangle /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileEntangle();    // 0x571CE0
    ~CProjectileEntangle() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x572290 -- Fire Storm (SPPR705, factory type 92/0x5C; the projectile is
// shared by SPWI081/SPWI399). Bare IcewindCProjectileSpellHit AOE leaf (own vtable
// 0x84F6B8; dtor ICF-folded onto 0x5768A0). Loads the firestorm burst into the first
// slot ("FStormX"/"EFF_P45") and the persistent fire area into the third
// ("FStormA"/"ARE_P03"), both copy-from-back. Lifetime (m_lifetime) 0x69, m_dirCount 1,
// m_aoeRange 200. Leaves m_visual2 empty.
class CProjectileFireStorm /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileFireStorm();    // 0x572290
    ~CProjectileFireStorm() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x571170 -- Acid Storm (SPWI708, factory type 211/0xD3). Bare
// IcewindCProjectileSpellHit AOE leaf (own vtable 0x84F274; dtor ICF-folded onto
// 0x5768A0). Loads the storm burst into the first slot ("AStormX", copy-from-back) and
// the persistent acid area into the third ("AStormA"/"ARE_M04"). Re-strike clock
// m_strikeInterval 10000, lifetime (m_lifetime) 0x2D, m_dirCount 1, m_aoeRange 200. Sets m_visual3AnimMode
// and m_visual3AnimFlag36 (not m_visual3RespawnFlag); leaves m_visual2 (and m_visual1.m_soundResRef) empty.
class CProjectileAcidStorm /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileAcidStorm();    // 0x571170
    ~CProjectileAcidStorm() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x5747A0 -- Spike Stones (SPPR512, factory type 213/0xD5). Bare
// IcewindCProjectileSpellHit AOE leaf (own vtable 0x84FFDC; dtor ICF-folded onto
// 0x5768A0). Loads the spike burst into the first slot ("SStoneA"/"EFF_P48") and the
// persistent spike area into the third (the same "SStoneA" cell + "ARE_P04");
// uniquely among the family it enables NO copy-from-back on either slot. Lifetime
// (m_lifetime) 0x4B0, m_aoeRange 0x96. Leaves m_visual2 empty.
class CProjectileSpikeStones /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileSpikeStones();    // 0x5747A0
    ~CProjectileSpikeStones() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x573E90 -- Power Word, Kill (SPWI903, factory type 278/0x116). Bare
// IcewindCProjectileSpellHit leaf (own vtable 0x84FD6C; dtor ICF-folded onto 0x5768A0).
// A single-burst spell-hit: loads only the first emission slot ("PWKillX"/"EFF_M39",
// copy-from-back) and no area slot. m_strikePeriod 10000, lifetime (m_lifetime) 0x2D, m_aoeRange
// 0x96.
class CProjectilePowerWordKill /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectilePowerWordKill();    // 0x573E90
    ~CProjectilePowerWordKill() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x5778A0 -- Symbol of Death (SPPR726, factory type 365/0x16D). Bare
// IcewindCProjectileSpellHit leaf (own vtable 0x850B70; dtor ICF-folded onto 0x5768A0).
// The minimal leaf of the family: loads only the first emission slot
// ("SoPainX"/"EFF_P49", copy-from-back) and sets m_aoeRange 300 -- nothing else, so every
// other field keeps the base-ctor default.
class CProjectileSymbolOfDeath /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileSymbolOfDeath();    // 0x5778A0
    ~CProjectileSymbolOfDeath() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x574A70 -- the Call Lightning / Static Charge strike bolt, created and
// fired per strike by IcewindCGameEffectCallLightning::ApplyEffect (0x564F80).
// Own vtable 0x850114 (dtor ICF-folded onto 0x5768A0); inherits Fire/AIUpdate/
// OnArrival from IcewindCProjectileSpellHit but OVERRIDES Strike (slot 37) so the
// bolt hits a single victim instead of the whole gather.  The ctor only sets
// m_aoeRange = 0x113; the per-strike visual/target/effects are filled by the
// engine (init 0x5704E0 sets the strike target-AI m_targetType, builder
// BuildSubSpellEffects loads the EffCL sub-spell's effects onto it).
class CProjectileCallLightningStrike /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileCallLightningStrike();    // 0x574A70
    ~CProjectileCallLightningStrike() override;   // slot 0; ICF-folded onto 0x5768A0
    void Strike(std::list<LONG>& targets) override;   // 0x574AA0 (vtable slot 37)
};

// Leaf 0x571310 -- Cloudkill (SPWI018, factory type 187/0xBB). Full three-slot cloud
// leaf in the Stinking Cloud mould (own vtable 0x84F310; dtor ICF-folded onto
// 0x5768A0): burst "CloudKX"/"RNG_M01", ring "CloudKR", and the persistent gas area
// "CloudKA"/"ARE_M02". m_lifetime 1000, m_aoeRange 0x96.
class CProjectileCloudkill /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileCloudkill();    // 0x571310
    ~CProjectileCloudkill() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x5714E0 -- Acid Fog (SPWI019, factory type 212/0xD4). Sibling of Cloudkill
// (own vtable 0x84F3AC): burst "DFogX"/"RNG_M01", ring "DFogR", gas area
// "DFogA"/"ARE_M02". m_lifetime 0x44C, m_aoeRange 0x96.
class CProjectileAcidFog /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileAcidFog();    // 0x5714E0
    ~CProjectileAcidFog() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x572670 -- Grease (SPWI101, factory type 101/0x65). Two-slot leaf (own vtable
// 0x84F7F0, like Web): burst "GreaseX"/"EFF_M31b" and the persistent grease area
// "GreaseA"/"ARE_M01" with m_visual3DensityBase 1000. m_lifetime 1000, m_aoeRange 100.
class CProjectileGrease /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileGrease();    // 0x572670
    ~CProjectileGrease() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x571930 -- Circle of Death (SPWI606, factory type 267/0x10B). Single-burst leaf
// (own vtable 0x84F448, like Power Word Kill): first slot only, "DSpellX"/"EFF_M42".
// m_strikePeriod 10000, m_lifetime 0x2D, m_aoeRange 300.
class CProjectileCircleOfDeath /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileCircleOfDeath();    // 0x571930
    ~CProjectileCircleOfDeath() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x573AF0 -- Insect Plague (SPPR510, factory type 216/0xD8). Full three-slot cloud
// leaf (own vtable 0x84FC34): burst "IPlaguX"/"RNG_P01", ring "IPlaguR", and the
// persistent swarm area "IPlaguA"/"ARE_P02". m_lifetime 0x5DC, m_aoeRange 0xFA.
class CProjectileInsectPlague /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileInsectPlague();    // 0x573AF0
    ~CProjectileInsectPlague() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x573600 -- Fiery Cloud (SPWI802, factory type 214/0xD6). Full three-slot cloud
// leaf (own vtable 0x84FB98) -- burst "ICloudX"/"EFF_M40", ring "ICloudR", persistent
// fire-cloud area "ICloudA"/"ARE_M05". The ONLY recovered leaf that sets
// m_visual3CloudFlag = 1, so it is the effect that drives the IcewindCSpellHitParticle
// cloud-flip (m_hasCloud / ICloudA-ICloudB) path. m_lifetime 0x41A, m_aoeRange 100.
class CProjectileFieryCloud /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileFieryCloud();    // 0x573600
    ~CProjectileFieryCloud() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x574140 -- Produce Fire (SPPR411, factory type 215/0xD7). Two-slot leaf (own
// vtable 0x84FE08): burst "PFireX"/"EFF_P45" and the persistent fire area
// "PFireA"/"ARE_P03". m_strikeInterval 10000, m_lifetime 100, m_aoeRange 0x3C.
class CProjectileProduceFire /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileProduceFire();    // 0x574140
    ~CProjectileProduceFire() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x575B90 -- Tremor (SPPR719, factory type 306/0x132). The most minimal leaf in
// the family (own vtable 0x850558): it sets only m_visual1.m_soundResRef ("ARE_P27")
// and m_aoeRange 0x15E -- no cell, no copy-from-back, every other field default.
class CProjectileTremor /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileTremor();    // 0x575B90
    ~CProjectileTremor() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x577B20 -- Dispel Magic (EFFDM1, factory type 246/0xF6). Single-burst leaf (own
// vtable 0x850C0C): first slot only, the abjuration glow "AbjuraX"/"ARE_M20",
// copy-from-back. m_aoeRange 0x96.
class CProjectileDispelMagic /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileDispelMagic();    // 0x577B20
    ~CProjectileDispelMagic() override;   // slot 0; ICF-folded onto 0x5768A0
};

// The enchantment-glow overlays -- four near-identical single-burst leaves that load
// the shared "EnchanX"/"ARE_M21" glow into the first emission slot (copy-from-back),
// strike once (m_strikePeriod 10000, m_strikeInterval 10, m_lifetime 0x2D) and differ
// only by RTTI/vtable and m_aoeRange.
//
// Leaf 0x574300 -- Sleep (SPWI116, factory type 42/0x2A; m_aoeRange 0x96).
class CProjectileSleep /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileSleep();    // 0x574300
    ~CProjectileSleep() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x572830 -- Hold Animal (SPPR305, factory type 249/0xF9; m_aoeRange 200).
class CProjectileHoldAnimal /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileHoldAnimal();    // 0x572830
    ~CProjectileHoldAnimal() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x572E60 -- Eye of Stone (SPIN132, factory type 190/0xBE; m_aoeRange 0x96).
class CProjectileEyeOfStone /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileEyeOfStone();    // 0x572E60
    ~CProjectileEyeOfStone() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x573160 -- Halt Undead (SPWI320, factory type 357/0x165; m_aoeRange 0x96).
class CProjectileHaltUndead /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileHaltUndead();    // 0x573160
    ~CProjectileHaltUndead() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x5745E0 -- Snilloc's Snowball Swarm (SPWI220, factory type 217/0xD9). A visible
// travelling leaf in the Fireball mould (own vtable 0x84FF40, no virtual overrides; dtor
// ICF-folded onto 0x5768A0): it flies the "SSSwarT" carrier cell, plays "TRA_18", loads
// the burst ("SSSwarX"/"RNG_M02") and ring ("SSSwarR") emission slots with copy-from-back,
// doubles the launch velocity and sets 16 facings. m_lifetime 0x2D, m_aoeRange 0xFA.
class CProjectileSnowballSwarm /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileSnowballSwarm();    // 0x5745E0
    ~CProjectileSnowballSwarm() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x572470 -- Flame Strike (projectile type 67, SPIN977; vtable 0x84F754).
// Sound-only overlay: just the "EFF_P16" impact sound on the burst slot, m_aoeRange 0x8C.
class CProjectileFlameStrike /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileFlameStrike();    // 0x572470
    ~CProjectileFlameStrike() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x572B30 -- Hold Monster (projectile type 263; vtable 0x84F928). The "EnchanX"/
// "ARE_M21" enchantment-glow overlay, identical to Hold Animal; m_aoeRange 200.
class CProjectileHoldMonster /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileHoldMonster();    // 0x572B30
    ~CProjectileHoldMonster() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x572110 -- Fire Seed (projectile type 270; vtable 0x84F61C). A visible travelling
// leaf: the "MagicStn" carrier cell bursts the "FSeedsX"/"EFF_P45" overlay, 16 facings.
// m_lifetime 0x2D, m_aoeRange 0x46.
class CProjectileFireSeed /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileFireSeed();    // 0x572110
    ~CProjectileFireSeed() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x573CC0 -- Malavon's Corrosive Fog (projectile type 279; vtable 0x84FCD0). A full
// three-slot cloud reusing the Death Fog visuals ("DFogX"/"DFogR"/"DFogA"), an Acid Fog
// sibling with a wider ring, a 2000-tick lifetime and m_aoeRange 0xFA.
class CProjectileCorrosiveFog /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileCorrosiveFog();    // 0x573CC0
    ~CProjectileCorrosiveFog() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x5756C0 -- Portal Animation Flipping Hack, Open (projectile type 294) and Close
// (type 297), one class for both. Not a spell (no SPL owner): a minimal portal-door
// overlay (vtable 0x850420) that runs the base ctor with lifetime 2000 and resets the
// target type to NOT_SPRITE (filter @0x8C76C8), adding no visuals.
class CProjectilePortalAnimFlip /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectilePortalAnimFlip();    // 0x5756C0
    ~CProjectilePortalAnimFlip() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x577590 -- Boulder, Big (Trap) (projectile type 383, SPWI088; vtable 0x850AD4). A
// visible travelling leaf (base lifetime 100): the "BIGBOLDR" carrier flies at velocity 7
// with one facing, looping "AM6103e"/"AM5101e", no burst overlay. m_aoeRange 100.
class CProjectileBigBoulder /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileBigBoulder();    // 0x577590
    ~CProjectileBigBoulder() override;   // slot 0; ICF-folded onto 0x5768A0
};

// Leaf 0x576990 -- Delayed Blast Fireball (SPWI714, projectile type 360/0x168; vtable
// 0x85099C). The only spell-hit leaf with its own data and virtual overrides: a Fireball-
// shaped carrier ("FirebaT" cell, "TRA_06" loop) with a full three-slot cloud, plus a
// proximity-delay state machine. OnArrival latches the bead instead of detonating;
// AIUpdate then rescans every fifth tick and fires the base detonation the moment any
// object enters a 100-unit radius. m_bBlasted/m_scanTimer follow the base m_miniB (binary
// 0x65E/0x660; their offsets drift with the VS2019 std::set size and nothing outside this
// class reads them).
class CProjectileDBFireball /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileDBFireball();    // 0x576990
    ~CProjectileDBFireball() override;   // slot 0; ICF-folded onto 0x5768A0
    void AIUpdate() override;   // vtable slot 3;  0x576BA0
    void OnArrival() override;  // vtable slot 28; 0x576CE0
private:
    BYTE m_bBlasted;   /*#guess*/ // ~0x65E -- latched once the delayed blast actually fires
    LONG m_scanTimer;  /*#guess*/ // ~0x660 -- proximity rescan counter (fires every 5th tick)
};

// Leaf 0x576CF0 -- Turn Undead (projectile type 376/0x178; vtable 0x850A38). A minimal
// no-visual spell-hit leaf (strike period 10000, interval 10, m_lifetime 0x2D, m_aoeRange
// 300); keeps the base AIUpdate/OnArrival, so unlike its code neighbour Delayed Blast
// Fireball it detonates on arrival.
class CProjectileTurnUndead /*#guess*/ : public IcewindCProjectileSpellHit {
public:
    CProjectileTurnUndead();    // 0x576CF0
    ~CProjectileTurnUndead() override;   // slot 0; ICF-folded onto 0x5768A0
};

// One cell of the detonation fan: a position (fixed point) and a flag the parent's
// AIUpdate toggles as the cell is consumed. The parent records these into a shared
// refcounted pool the IcewindCSpellHitParticle children reference. Names are
// guesses; the 10-byte element matches the parent's /10 pool stride.
#pragma pack(push, 2)
struct IcewindCSpellHitCell /*#guess*/ {
    /* 0x00 */ LONG  x;
    /* 0x04 */ LONG  y;
    /* 0x08 */ SHORT flag;
};
#pragma pack(pop)
static_assert(sizeof(IcewindCSpellHitCell) == 0xA,
    "IcewindCSpellHitCell must be the 10-byte element the fan pool stores");

// Refcounted shared pool of fan cells (m_emission2.m_cellPool; children hold a
// counted reference via m_cellPool/field_104). The cell vector lives at +0x04 and
// the refcount at +0x10. Provisional layout -- the +0 field and the pool's
// creation are recovered with OnArrival 0x56F410.
struct IcewindCSpellHitCellPool /*#guess*/ {
    IcewindCSpellHitCellPool() : field_0(0), m_refCount(0) {}   // 0x5868E0

    /* 0x00 */ LONG field_0;
    /* 0x04 */ std::vector<IcewindCSpellHitCell> m_cells;
    /* 0x10 */ LONG m_refCount;
};

// Spell-hit emission-slot descriptor (ctor 0x56FDC0, sizeof 0x32). One visual
// layer of the detonation: a resref name slot (inline {ptr,len,cap} ResName), a
// second name slot, the layer's IcewindCVisualEffect tint/transparency params,
// the anim-mode selector and a spawn-budget sentinel. The spell-hit projectile
// carries these as its emission slots and OnArrival copies them into the visual.
// Member names recovered via Frida (tmp_spellhit_naming_trace.py): the resref
// slots are inline {CString ptr, LONG len, LONG cap} (trace: len=7, cap=31).
#pragma pack(push, 1)
struct IcewindCSpellHitEmission /*#guess*/ {
    IcewindCSpellHitEmission();   // 0x56FDC0

    /* 0x00 */ BYTE    m_resref0Flags;  // ResName flags byte
    /* 0x01 */ BYTE    _pad1[3];
    /* 0x04 */ char*   m_resref0;       // ResName::m_pName (refcounted block+1; NULL=empty). POD: the binary copies the slot flat (rep movs @0x56C308), so this must NOT be a CString (its throwing operator= aborts).
    /* 0x08 */ INT     m_resref0Len;    // resref0 name length (Frida: 7)
    /* 0x0C */ INT     m_resref0Cap;    // resref0 name capacity (Frida: 31)
    /* 0x10 */ BYTE    m_resref1Flags;  // ResName flags byte
    /* 0x11 */ BYTE    _pad11[3];
    /* 0x14 */ char*   m_resref1;       // ResName::m_pName (refcounted block+1; NULL=empty)
    /* 0x18 */ INT     m_resref1Len;    // resref1 name length
    /* 0x1C */ INT     m_resref1Cap;    // resref1 name capacity
    /* 0x20 */ IcewindCVisualEffect m_visualEffect;
    /* 0x2C */ BYTE    m_animMode;      // -> CGameAnimationTypeEffect.m_animMode (gate ==1)
    /* 0x2D */ BYTE    _pad2D;
    /* 0x2E */ INT     m_maxMovingSpawn; // ctor 0x7FFFFFFF; moving-spawn budget (Frida: 20)
};
#pragma pack(pop)
static_assert(sizeof(IcewindCSpellHitEmission) == 0x32,
    "IcewindCSpellHitEmission must match the IWD2.exe 0x56FDC0 layout (0x32)");

// The richer emission-slot variant (ctor 0x56FE30, sizeof 0x4E): the same 0x2C
// prefix plus the shared cell pool and a trailing density/timing block. Used for
// the third spell-hit emission slot (Fireball's stationary-ember source + range
// ring). Frida-confirmed Fireball values: m_densityBase 250, m_emitPeriod 6,
// m_densityRampDiv 30, m_respawnFlag 1, m_animMode 1, m_animFlag36 1, m_cloudFlag 0.
#pragma pack(push, 1)
struct IcewindCSpellHitEmissionRanged /*#guess*/ {
    IcewindCSpellHitEmissionRanged();   // 0x56FE30

    /* 0x00 */ BYTE    m_resref0Flags;  // ResName flags byte
    /* 0x01 */ BYTE    _pad1[3];
    /* 0x04 */ char*   m_resref0;       // ResName::m_pName (refcounted block+1; NULL=empty). POD: the binary copies the slot flat (rep movs @0x56C308), so this must NOT be a CString (its throwing operator= aborts).
    /* 0x08 */ INT     m_resref0Len;    // resref0 name length (Frida: 7)
    /* 0x0C */ INT     m_resref0Cap;    // resref0 name capacity (Frida: 31)
    /* 0x10 */ BYTE    m_resref1Flags;  // ResName flags byte
    /* 0x11 */ BYTE    _pad11[3];
    /* 0x14 */ char*   m_resref1;       // ResName::m_pName (refcounted block+1; NULL=empty)
    /* 0x18 */ INT     m_resref1Len;    // resref1 name length
    /* 0x1C */ INT     m_resref1Cap;    // resref1 name capacity
    /* 0x20 */ IcewindCVisualEffect m_visualEffect;
    /* 0x2C */ IcewindCSpellHitCellPool* m_cellPool;   // ctor: NULL; shared fan-cell pool, filled by OnArrival
    /* 0x30 */ INT     m_lastCellIndex; // ctor 0; scratch: index of the just-pushed cell (-> particle m_cellIndex)
    /* 0x34 */ BYTE    m_respawnFlag;   // -> particle m_respawnFromPool (Frida: 1)
    /* 0x35 */ BYTE    m_animMode;      // -> CGameAnimationTypeEffect.m_animMode (gate ==1; Frida: 1)
    /* 0x36 */ BYTE    m_animFlag36;    // -> CGameAnimationTypeEffect.m_animFlag36 (Frida: 1)
    /* 0x37 */ BYTE    _pad37;
    /* 0x38 */ INT     m_densityBase;   // ctor 250; spawn-density base threshold
    /* 0x3C */ INT     m_emitPeriod;    // ctor 6; m_emitCooldown reload
    /* 0x40 */ INT     m_densityRampDiv;// ctor 30; age^2 divisor in the density ramp
    /* 0x44 */ BYTE    m_cloudFlag;     // -> particle m_hasCloud (ICloudA gate; Frida: 0)
    /* 0x45 */ BYTE    _pad45;
};
#pragma pack(pop)
static_assert(sizeof(IcewindCSpellHitEmissionRanged) == 0x46,
    "IcewindCSpellHitEmissionRanged must match the IWD2.exe 0x56FE30 layout (0x46)");

// CGameObject leaf 0x56BF30 -- the on-ground detonation visual that
// IcewindCProjectileSpellHit::OnArrival spawns when the projectile arrives. It
// is NOT a CProjectile: its own vtable 0x84F0DC has exactly CGameObject's 27
// slots, overriding only the destructor, AIUpdate, RemoveFromArea and Render
// (slots 1/13/25 -- GetObjectType, IsOver, EvaluateStatusTrigger -- are the
// inherited folded CGameObject stubs). It is a free-standing area animation
// object that draws a BAM through an embedded CVidCell and expands a radial fan
// of animation cells outward from the impact at the projectile's velocity,
// fading over m_duration ticks.
//
// OnArrival builds it from the spell-hit projectile's three "emission slot"
// visual descriptors (proj +0x4E2 / +0x50E / +0x540; e.g. Fireball's
// FirebaX / RNG_M03 / FirebaR detonation + range graphics) plus the impact
// position, the launch velocity and the m_lifetime lifetime. The ctor loads the
// detonation BAM (CDimm::GetResObject type 1000), copies the other two
// descriptors into field_210/field_242, computes the per-direction velocity
// table, registers the object (CGameObjectArray::Add) and adds it to the area.
//
// Name is a guess: IWD2.exe carries no RTTI and the BG2 PDB has no match (this
// is an IWD2-specific Icewind class). vtable 0x84F0DC, ctor 0x56BF30.
//
// SCAFFOLD: the layout below is recovered from the 0x56BF30 ctor (sub-object
// offsets asm-confirmed); the method bodies are faithful stubs pending recovery
// of 0x56BF30 (ctor), 0x56D0A0 (AIUpdate), 0x56D730 (Render), 0x56D9B0
// (RemoveFromArea) and 0x56CEE0 (dtor). Not yet wired into OnArrival.
#pragma pack(push, 2)
class IcewindCSpellHitVisual /*#guess*/ : public CGameObject {
public:
    // Args mirror the 0x56BF30 __thiscall: the three emission-slot descriptors
    // (emission0 supplies the detonation BAM + its visual-effect params;
    // emission1/emission2 are copied in whole), the area, the impact point, the
    // range/frame seed, the launch velocity, a global byte (DAT_0085BD6D) and
    // the lifetime.
    IcewindCSpellHitVisual(const IcewindCSpellHitEmission& emission0,
                           const IcewindCSpellHitEmission& emission1,
                           const IcewindCSpellHitEmissionRanged& emission2, CGameArea* pArea,
                           const CPoint& pos, SHORT nRange, BYTE nVelocity, BYTE a8,
                           SHORT nDuration);   // 0x56BF30
    ~IcewindCSpellHitVisual() override;   // 0x56CEE0 (vtable slot 0)

    void AIUpdate() override;        // 0x56D0A0 (slot 3)
    void RemoveFromArea() override;  // 0x56D9B0 (slot 18)
    void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;   // 0x56D730 (slot 19)

    /* 006E */ BYTE m_terrainTable[16];   // seeded from CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE
    /* 007E */ SHORT field_7E;            // reserved: ctor 0, never read (Frida)
    /* 0080 */ SHORT field_80;            // reserved: ctor 0, never read (Frida)
    /* 0082 */ BYTE field_82[8];          // reserved: never read (Frida: const 30 20 ..)
    /* 008A */ CVidCell m_cell;           // detonation BAM cell (sub-ctor 0x7ACD70)
    /* 0164 */ CVidPalette m_palette;     // sub-ctor 0x7BEEA0 (nType = DAT_0085E84A)
    /* 0188 */ SHORT m_duration;          // = m_lifetime lifetime
    /* 018A */ BYTE m_frameCount;         // (nRange-1)/nVelocity + 1
    /* 018B */ BYTE m_collision;          // = a8; AIUpdate wall-bounce mode (COLLISION_DESTROY/REBOUND)
    // Radial velocity fan: the coverage bitmap half-dimensions + arc run lengths,
    // and the malloc'd coverage map / cell (16 bytes/entry) / velocity buffers the
    // ctor fills (0x18C..0x1A7). Frida: m_coverHalfW == m_coverHalfH == 13.
    /* 018C */ LONG m_coverHalfW;
    /* 0190 */ LONG m_coverHalfH;
    /* 0194 */ void* m_coverageMap;       // (m_coverHalfW*2+1)*(m_coverHalfH*2+1) bytes
    /* 0198 */ LONG m_arcLen1;            // GetEllipseArcPixelList run 1
    /* 019C */ LONG m_arcLen2;            // GetEllipseArcPixelList run 2; loop (m_arcLen1+m_arcLen2)*4
    /* 01A0 */ void* m_fanCells;          // 16-byte cell entries (x, y, vel)
    /* 01A4 */ void* m_fanVel;            // per-entry velocity table
    /* 01A8 */ CSound m_sound;            // sub-ctor 0x7A8BB0
    /* 020C */ CString field_20C;         // reserved: empty CString, never assigned (Frida: afxNil)
    /* 0210 */ IcewindCSpellHitEmission m_emission1;        // sub-ctor 0x56FDC0
    /* 0242 */ IcewindCSpellHitEmissionRanged m_emission2;  // sub-ctor 0x56FE30
    /* 0288 */ INT m_movingSpawnCount;    // ctor 0; covered-pixel counter, capped at m_emission1.m_maxMovingSpawn
    /* 028C */ INT m_age;                 // ctor 0; ++ per tick; drives the density ramp (age^2)
    /* 0290 */ IcewindCVisualEffect m_visualEffect;   // sub-ctor 0x586A40
    /* 029C */ LONG m_emitCooldown;       // stationary-spawn countdown; reload = m_emission2.m_emitPeriod (6)
    /* 02A0 */ BYTE m_bamLoaded;          // 1 iff emission0 has a resref; Render gates on it
    /* 02A1 */ BYTE _pad2A1;
};
#pragma pack(pop)
// Validate the OWN-field span (0x6E..0x2A2 in the binary), not the absolute
// size: the compiled CGameObject base is 0x78 here vs 0x6E in IWD2.exe (the base
// header carries no #pragma pack(2), an existing whole-codebase discrepancy), so
// every CGameObject leaf is shifted up by that excess. This assert still pins our
// members to the binary's 0x234-byte tail and survives a future base-layout fix.
static_assert(sizeof(IcewindCSpellHitVisual) - sizeof(CGameObject) == 0x2A2 - 0x6E,
    "IcewindCSpellHitVisual own-field span must match the IWD2.exe 0x56BF30 layout (0x6E..0x2A2)");

// CGameObject leaf 0x56E280 -- one travelling particle of the detonation fan.
// IcewindCSpellHitVisual::AIUpdate spawns a ring of these outward from the
// impact: each carries a single BAM animation cell that flies along the launch
// velocity, bounces off walls (CSearchBitmap::GetLOSCost) and expires after a
// few frames, producing the spreading "spray" of the ground VFX.
//
// Like its parent it is NOT a CProjectile: its own vtable 0x84F14C has exactly
// CGameObject's 27 slots, overriding only the destructor (0x56E260), AIUpdate
// (0x56E650), RemoveFromArea (0x56ECF0) and Render (0x56EA90). The 0x84F14C
// table is immediately followed in .rdata by the embedded animator's vtable
// 0x84F1B8 (= 0x84F14C + 27*4), which a raw vtable scan can misread as a 28th
// slot -- there is no added virtual.
//
// It embeds a small CGameAnimation-derived BAM animator at +0x7E (its own
// vtable 0x84F1B8, slot 0 = CalculateGCBoundsRect at 0x56E210 forwarding to the
// sub-object's m_animation at +0x82). That sub-object's full layout is a
// separate recovery; it is left as an opaque span here.
//
// Two sibling ctors build it from the two cell-spawn paths of the parent's
// AIUpdate: 0x56E280 (decodes the animation via FUN_0055D3A0) and 0x56DF00
// (via FUN_0055CD70). Only 0x56E280 is scaffolded below; the 0x56DF00 overload
// is recovered later (its descriptor argument type fixes the overload split).
//
// Name is a guess (no RTTI; no BG2 PDB match -- IWD2-specific Icewind class).
// vtable 0x84F14C, ctor 0x56E280, sizeof 0x112.
//
// SCAFFOLD: layout below is recovered from the 0x56E280 ctor (offsets
// asm/decompile-confirmed); method bodies are faithful stubs pending recovery
// of 0x56E280/0x56DF00 (ctors), 0x56E650 (AIUpdate), 0x56EA90 (Render),
// 0x56ECF0 (RemoveFromArea) and 0x56E260 (dtor). Not yet spawned by the parent.
#pragma pack(push, 2)
class IcewindCSpellHitParticle /*#guess*/ : public CGameObject {
public:
    // Two ctors, one per emission slot the parent's AIUpdate spawns from. Args
    // mirror the __thiscall: the spawn descriptor, the area, the impact point, an
    // AddToArea insert flag, the launch velocity (also seeds the BAM direction via
    // CGameSprite::GetDirection) and the two mode bytes the parent's wall-bounce
    // logic reads back. The descriptor type is the overload split: the
    // IcewindCSpellHitEmissionRanged form (m_emission2) also references a shared
    // cell object via field_2C; the plainer IcewindCSpellHitEmission form
    // (m_emission1) does not.
    IcewindCSpellHitParticle(const IcewindCSpellHitEmission& descriptor, CGameArea* pArea,
                             const CPoint& pos, int a5 /*#guess*/, const CPoint& velocity,
                             SHORT a7, BYTE mode8, BYTE mode9);   // 0x56DF00
    IcewindCSpellHitParticle(const IcewindCSpellHitEmissionRanged& descriptor, CGameArea* pArea,
                             const CPoint& pos, int a5 /*#guess*/, const CPoint& velocity,
                             SHORT a7, BYTE mode8, BYTE mode9);   // 0x56E280
    ~IcewindCSpellHitParticle() override;   // 0x56E260 (vtable slot 0)

    void AIUpdate() override;        // 0x56E650 (slot 3)
    void RemoveFromArea() override;  // 0x56ECF0 (slot 18)
    void Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface) override;   // 0x56EA90 (slot 19)

    /* 006E */ BYTE m_terrainTable[16];   // seeded from CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE
    // The 0x6E..0xA0 block is laid out exactly like CGameTemporal -- the engine's
    // other "play a CGameAnimation drifting along a velocity until it expires"
    // CGameObject. m_animation wraps the detonation effect (m_animation.m_animation
    // is the 0x55D3A0/0x55CD70 CGameAnimationTypeEffect, vtable 0x84F1B8 slot 0
    // CalculateGCBoundsRect at 0x56E210 forwards to it).
    /* 007E */ CGameAnimation m_animation;
    /* 0088 */ INT    m_animationRunning;
    /* 008C */ CPoint m_posExact;
    /* 0094 */ CPoint m_posDelta;
    /* 009C */ SHORT  m_duration;
    /* 009E */ BYTE   m_durationFade;       // 0x56E280 param a8; read back by the parent's wall-bounce logic
    /* 009F */ BYTE   m_collision;          // 0x56E280 param a9
    /* 00A0 */ CSound m_sound;            // CResHelper<CResWave,4>; impact sound from descriptor.m_resref1, channel 0xE
    /* 0104 */ IcewindCSpellHitCellPool* m_cellPool;   // shared fan-cell pool (= descriptor.m_cellPool)
    /* 0108 */ INT   m_cellIndex;         // = descriptor.m_lastCellIndex (Frida: climbs 38..119)
    /* 010C */ BYTE  m_respawnFromPool;   // = descriptor.m_respawnFlag (stationary: 1, moving: 0)
    /* 010D */ BYTE  m_hasCloud;          // = descriptor.m_cloudFlag (ICloudA gate; Frida: 0)
    /* 010E */ INT   m_animTick;          // ctor 0; ++ per AIUpdate tick
};
#pragma pack(pop)
// No exact-size static_assert: like CGameTemporal this embeds CGameAnimation and
// CSound by value, whose compiled sizes carry the same base-layout slack as the
// rest of the codebase (see the IcewindCSpellHitVisual 0x6E-vs-0x78 note above).
// The /* 0xNNN */ comments pin the IWD2.exe offsets.

// ---------------------------------------------------------------------------
// CProjectileCallLightning (SPPR302, vtable 0x84E8C0)
//
// Instant-effect spell projectile for Call Lightning and its variants
// (CallLiH / FStrikH / SunscoH beams).  Fire calls DeliverEffects immediately,
// then the object lives as a visual marker for m_lifetime ticks doing its OWN
// BAM frame animation -- it is NOT a CProjectileBAM: it carries an inline
// CVidCell (+0x192) plus a range CVidPalette (+0x26C), with no CVidBitmap or
// IcewindCVisualEffect.  0x29A bytes in VC6; derives from CProjectileInstant
// (BG2 PDB) and adds the cell/palette and three trailing fields.
// ---------------------------------------------------------------------------
#pragma pack(push, 2)
class CProjectileCallLightning : public CProjectileInstant {
public:
    CProjectileCallLightning(const CResRef& resRef, const CResRef& soundRef,
                             BYTE colorIndex, LONG renderFlag, LONG animFlag,
                             SHORT lifetime);   // 0x5348C0
    void AIUpdate() override;      // 0x534C10 (slot 3)
    void Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos,
              LONG nHeight, SHORT nType) override;   // 0x535100 (slot 27)
    void Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface) override;   // 0x534DD0 (slot 19)

private:
    /* 0192 */ CVidCell    m_vidCell;
    /* 026C */ CVidPalette m_palette;
    /* 0290 */ LONG        m_renderFlag;   // ctor param5: Render blend -- 0 => FXPREP_CLEARFILL, else FXPREP_COPYFROMBACK
    /* 0294 */ LONG        m_animFlag;     // ctor param6: AIUpdate frame gate -- 0 => sequential advance, else random flicker
    /* 0298 */ SHORT       m_lifetime;     // ctor projType: tick counter (0x14 CallLiH / 0x1C FStrikH / 0x18 SunscoH)
};
#pragma pack(pop)

#endif /* CPROJECTILE_H_ */
