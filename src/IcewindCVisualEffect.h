#ifndef ICEWINDCVISUALEFFECT_H_
#define ICEWINDCVISUALEFFECT_H_

#include "mfc.h"

class IcewindCVisualEffect {
public:
    IcewindCVisualEffect();
    void SetTintFromFlags(BOOLEAN a1);
    void SetCopyFromBack(BOOLEAN a1);
    void SetTransparency(BOOLEAN a1, INT nTransValue);

    /* 0000 */ BOOLEAN m_bTintEnabled;
    /* 0001 */ BOOLEAN m_bCopyFromBack;
    /* 0002 */ BOOLEAN field_2;
    /* 0003 */ BOOLEAN m_bTransparent;
    /* 0004 */ INT m_nTransValue;
    /* 0008 */ DWORD m_dwFlags;
};

#endif /* ICEWINDCVISUALEFFECT_H_ */
