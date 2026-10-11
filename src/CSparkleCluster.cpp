#include "CSparkleCluster.h"

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameObjectArray.h"
#include "CInfGame.h"
#include "CParticle.h"
#include "CRuleTables.h"
#include "CUtil.h"
#include "CVidInf.h"
#include <cstdio>
#include <cstdlib>

// 0x84D600
const USHORT CSparkleCluster::TYPE_TRAVEL = 1;
// 0x84D602
const USHORT CSparkleCluster::TYPE_CONJURE = 2;
// 0x84D604
const USHORT CSparkleCluster::TYPE_EXPLOSION = 4;
// 0x84D5F0
const USHORT CSparkleCluster::DEFAULT_CONJURE_DURATION = 25;

// 0x51AE40
CSparkleCluster::CSparkleCluster(SHORT colorType, USHORT type, USHORT conjureDuration)
    : m_particleList(10)
{
    m_nTimer = 0;
    m_pVidMode = g_pBaldurChitin->GetCurrentVideoMode();
    SetRect(&m_rBounding, 0, 0, 0, 0);
    m_wType = type;
    m_nConjureDuration = conjureDuration;
    m_nGravity = 50;
    const C2DArray& colors = g_pBaldurChitin->GetObjectGame()->GetRuleTables().m_tSparkleColors;
    static const SHORT rows[] = {5, 0, 6, 1, 8, 2, 7, 9, 3, 4, 10, 11};
    if (colorType < 1 || colorType > 12) {
        UTIL_ASSERT(FALSE);
        return;
    }
    COLORREF* palette[] = {&m_colorStart, &m_colorFade1, &m_colorFade2, &m_colorFade3, &m_colorFade4};
    for (LONG i = 0; i < 5; ++i) {
        UINT red, green, blue;
        sscanf(colors.GetAt(CPoint(i, rows[colorType - 1])), "RGB(%u,%u,%u)", &red, &green, &blue);
        *palette[i] = RGB(red, green, blue);
    }
}
// 0x51C330
CSparkleCluster::~CSparkleCluster()
{
    POSITION pos = m_particleList.GetHeadPosition();
    while (pos != NULL) {
        delete m_particleList.GetNext(pos);
    }
    m_particleList.RemoveAll();
}
// 0x51C3D0
void CSparkleCluster::AddToArea(CGameArea* area, const CPoint& pos, LONG height, BYTE listType)
{
    LONG gravity = m_nGravity;
    m_fade1Height = static_cast<BYTE>(static_cast<LONG>(static_cast<DWORD>(height) * 99) / 100);
    m_fade2Height = static_cast<BYTE>(static_cast<LONG>(static_cast<DWORD>(height) * 85) / 100);
    m_fade3Height = static_cast<BYTE>(static_cast<LONG>(static_cast<DWORD>(height) * 75) / 100);
    m_fade4Height = static_cast<BYTE>(static_cast<LONG>(static_cast<DWORD>(height) * 55) / 100);
    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE)
        != CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }
    CGameObject::AddToArea(area, pos, height, listType);
    if ((m_wType & TYPE_EXPLOSION) != 0) {
        GenerateExplosion();
        return;
    }
    LONG x, y;
    if ((m_wType & TYPE_CONJURE) != 0) {
        x = m_pos.x + 3;
        y = m_pos.y - 5;
        m_nTimer = 0;
        SetRect(&m_rBounding, max(m_pos.x - 13, 0), max(m_pos.y - height - 13, 0),
            m_pos.x + 13, m_pos.y + 13);
    } else {
        x = m_pos.x - 5;
        y = m_pos.y - 5;
        SetRect(&m_rBounding, max(x, 0), max(y - height, 0), m_pos.x + 4, m_pos.y + 4);
    }
    LONG z = static_cast<LONG>(static_cast<DWORD>(height) << CParticle::RESOLUTION_INC);
    for (SHORT i = 1; i < 15; ++i) {
        LONG dx = rand() % 10;
        LONG dy = rand() % 10;
        LONG px = static_cast<LONG>(static_cast<DWORD>(x + dx) << CParticle::RESOLUTION_INC);
        LONG py = static_cast<LONG>(static_cast<DWORD>(y + dy) * 4) / 3;
        py = static_cast<LONG>(static_cast<DWORD>(py) << CParticle::RESOLUTION_INC);
        COLORREF color = (dx & 1) != 0 ? m_colorFade1 : m_colorStart;
        color = g_pChitin->GetCurrentVideoMode()->ApplyFadeAmount(color);
        color = g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(color);
        m_particleList.AddTail(new CParticle(px, py, z, (i & 1) != 0 ? 4 : -4,
            0, 0, color, 0, gravity, CParticle::CONSTANT_VELOCITY));
        gravity += 10;
    }
}
// 0x51C6E0
void CSparkleCluster::AIUpdate()
{
    UTIL_ASSERT(!m_particleList.IsEmpty());
    POSITION pos = m_particleList.GetHeadPosition();
    while (pos != NULL) {
        POSITION old = pos;
        CParticle* particle = m_particleList.GetNext(pos);
        LONG height = particle->m_pos.z >> CParticle::RESOLUTION_INC;
        COLORREF color;
        BOOL fade = TRUE;
        if (height < m_fade4Height) color = m_colorFade4;
        else if (height < m_fade3Height) color = m_colorFade3;
        else if (height < m_fade2Height) color = m_colorFade2;
        else if (height < m_fade1Height) color = m_colorFade1;
        else fade = FALSE;
        if (fade) {
            INT scale = (height & 3) + 1;
            particle->m_rgbColor = RGB(min(GetRValue(color) * scale, 255),
                min(GetGValue(color) * scale, 255), min(GetBValue(color) * scale, 255));
        }
        if ((m_wType & TYPE_EXPLOSION) == 0 && (rand() & 4) != 0) {
            particle->m_vel.x = static_cast<LONG>(0U - static_cast<DWORD>(particle->m_vel.x));
            particle->m_vel.y = static_cast<LONG>(static_cast<DWORD>(particle->m_vel.y) * 3) >> 2;
        }
        LONG velocity = static_cast<LONG>(static_cast<DWORD>(particle->m_vel.z) - particle->m_nGravity);
        particle->m_pos.z = max(static_cast<LONG>(static_cast<DWORD>(particle->m_pos.z) + velocity), 0);
        if (particle->m_pos.z > 0) {
            particle->m_pos.x = static_cast<LONG>(static_cast<DWORD>(particle->m_pos.x) + particle->m_vel.x);
            particle->m_pos.y = static_cast<LONG>(static_cast<DWORD>(particle->m_pos.y) + particle->m_vel.y);
            particle->m_vel.z = velocity;
        } else {
            m_particleList.RemoveAt(old);
            delete particle;
            if (m_particleList.IsEmpty()) {
                RemoveFromArea();
                if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                        CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE) == CGameObjectArray::SUCCESS) {
                    delete this;
                }
                return;
            }
        }
    }
    if ((m_wType & TYPE_CONJURE) != 0 && m_nTimer < m_nConjureDuration) {
        static const SHORT offsets[8][2] = {{3,-5}, {5,5}, {-5,3}, {-15,5},
            {-13,-5}, {-15,-15}, {-5,-13}, {5,-15}};
        DWORD direction = m_nTimer++ & 7;
        LONG x = m_pos.x + offsets[direction][0];
        LONG y = m_pos.y + offsets[direction][1];
        LONG z = static_cast<LONG>(static_cast<DWORD>(m_posZ) << CParticle::RESOLUTION_INC);
        COLORREF color = g_pChitin->GetCurrentVideoMode()->ApplyFadeAmount(m_colorStart);
        color = g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(color);
        SHORT gravity = 50;
        for (SHORT i = 1; i < 15; ++i) {
            LONG px = x + static_cast<USHORT>(rand() % 10);
            LONG py = y + static_cast<USHORT>(rand() % 10);
            px = static_cast<LONG>(static_cast<DWORD>(px) << CParticle::RESOLUTION_INC);
            py = static_cast<LONG>(static_cast<DWORD>(py) * 4) / 3;
            py = static_cast<LONG>(static_cast<DWORD>(py) << CParticle::RESOLUTION_INC);
            m_particleList.AddTail(new CParticle(px, py, z, (i & 1) != 0 ? 4 : -4,
                0, 0, color, 0, gravity, CParticle::CONSTANT_VELOCITY));
            gravity += 10;
        }
    }
}
// 0x51CC00
void CSparkleCluster::Render(CGameArea* area, CVidMode* vidMode, int surface)
{
    CInfinity* infinity = m_pArea->GetInfinity();
    CRect viewport(infinity->nCurrentX, infinity->nCurrentY,
        infinity->nCurrentX + infinity->rViewPort.Width(), infinity->nCurrentY + infinity->rViewPort.Height());
    DWORD flags = CInfinity::FXPREP_CLEARFILL | 1;
    SHORT tile = static_cast<SHORT>((m_pos.y / 32) * m_pArea->m_visibility.m_nWidth + m_pos.x / 32);
    if (!m_pArea->m_visibility.IsTileVisible(tile)
        || !IntersectRect(&viewport, &viewport, &m_rBounding)) {
        return;
    }
    CRect fx;
    SetRect(&fx, 0, 0, m_rBounding.Width(), m_rBounding.Height());
    CPoint zero(0, 0);
    infinity->FXPrep(fx, flags, -1, zero, zero);
    CPoint origin(m_pos.x - (m_rBounding.Width() >> 1), m_pos.y - m_posZ);
    if (m_wType == TYPE_CONJURE) origin.y -= 8;
    else if (m_wType == TYPE_EXPLOSION) origin.y -= 50;
    if (infinity->FXLock(fx, flags)) {
        POSITION pos = m_particleList.GetHeadPosition();
        while (pos != NULL) {
            CParticle* particle = m_particleList.GetNext(pos);
            COLORREF old = particle->m_rgbColor;
            particle->m_rgbColor = g_pChitin->GetCurrentVideoMode()->ApplyFadeAmount(old);
            particle->m_rgbColor = g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(particle->m_rgbColor);
            static_cast<CVidInf*>(infinity->pVidMode)->FXRender(particle, m_rBounding, CParticle::DOTS, 5);
            particle->m_rgbColor = old;
        }
        infinity->FXRenderClippingPolys(m_pos.x, m_pos.y, 0, zero, m_rBounding, FALSE, flags);
        infinity->FXUnlock(flags, NULL, zero);
        infinity->FXBltFrom(surface, fx, origin.x, origin.y, 0, 0, flags);
    }
}
// 0x51CE50
void CSparkleCluster::GenerateExplosion()
{
    LONG dx = static_cast<LONG>(static_cast<DWORD>(m_explosionDir.x) - m_pos.x);
    LONG dy = static_cast<LONG>(static_cast<DWORD>(m_explosionDir.y) - m_pos.y);
    auto multiply = [](LONG n, DWORD factor) { return static_cast<LONG>(static_cast<DWORD>(n) * factor); };
    SHORT direction;
    if (m_explosionDir.y >= m_pos.y) {
        if (m_explosionDir.x >= m_pos.x) {
            direction = 12;
            if (multiply(dy, 27) > multiply(dx, 16)) {
                direction = 14;
                if (multiply(dy, 7) > multiply(dx, 38)) direction = 0;
            }
        } else {
            dx = static_cast<LONG>(0U - static_cast<DWORD>(dx));
            direction = 4;
            if (multiply(dy, 27) > multiply(dx, 16)) {
                direction = 2;
                if (multiply(dy, 7) > multiply(dx, 38)) direction = 0;
            }
        }
    } else {
        dy = static_cast<LONG>(0U - static_cast<DWORD>(dy));
        if (m_explosionDir.x >= m_pos.x) {
            direction = 12;
            if (multiply(dy, 27) > multiply(dx, 16)) {
                direction = 10;
                if (multiply(dy, 7) > multiply(dx, 38)) direction = 8;
            }
        } else {
            dx = static_cast<LONG>(0U - static_cast<DWORD>(dx));
            direction = 4;
            if (multiply(dy, 27) > multiply(dx, 16)) {
                direction = 6;
                if (multiply(dy, 7) > multiply(dx, 38)) direction = 8;
            }
        }
    }
    LONG y = m_pos.y - m_posZ;
    SetRect(&m_rBounding, max(m_pos.x - 50, 0), max(y - 50, 0), m_pos.x + 50, y + 50);
    static const SHORT velocities[8][6] = {
        {700,700, 0,1000, -700,700},
        {0,1000, -700,700, -1000,0},
        {-700,700, -1000,0, -700,-700},
        {-1000,0, -700,-700, 0,-1000},
        {-700,-700, 0,-1000, 700,-700},
        {0,-1000, 700,-700, 1000,0},
        {700,-700, 1000,0, 700,700},
        {1000,0, 700,700, 0,1000},
    };
    const SHORT* velocity = velocities[direction / 2];
    LONG x = m_pos.x - 10;
    y = m_pos.y - 10;
    LONG z = static_cast<LONG>(static_cast<DWORD>(m_posZ) << CParticle::RESOLUTION_INC);
    COLORREF color = g_pChitin->GetCurrentVideoMode()->ApplyFadeAmount(m_colorStart);
    color = g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(color);
    for (INT stream = 0; stream < 3; ++stream) {
        LONG gravity = 50;
        for (INT i = 0; i < 14; ++i) {
            LONG px = x + 2 * (rand() % 10);
            LONG py = y + 2 * (rand() % 10);
            px = static_cast<LONG>(static_cast<DWORD>(px) << CParticle::RESOLUTION_INC);
            py = static_cast<LONG>(static_cast<DWORD>(py) * 4) / 3;
            py = static_cast<LONG>(static_cast<DWORD>(py) << CParticle::RESOLUTION_INC);
            m_particleList.AddTail(new CParticle(px, py, z, velocity[stream * 2],
                velocity[stream * 2 + 1], 1000, color, 0, gravity, CParticle::CONSTANT_VELOCITY));
            gravity += 10;
        }
    }
}
