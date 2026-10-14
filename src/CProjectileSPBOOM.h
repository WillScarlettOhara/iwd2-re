#ifndef CPROJECTILESPBOOM_H_
#define CPROJECTILESPBOOM_H_

#include "CProjectile.h"

// Name from the default BAM; no BG2 symbol. Binary vtable 0x84DFA4.
#pragma pack(push, 2)
class CProjectileSPBOOM /*#guess*/ : public CProjectileTravelling {
public:
    CProjectileSPBOOM();
    void AIUpdate() override;
    virtual void AddVisualToArea(CGameArea* area, CPoint point);
};
#pragma pack(pop)

#endif
