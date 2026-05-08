#ifndef CUNKNOWN2_H_
#define CUNKNOWN2_H_

#include "mfc.h"

// Unknown class in `CSoundMixer`.
class CUnknown2 {
public:
    CUnknown2();
    ~CUnknown2();
    BOOL sub_799C90();
    void sub_7C25B0();

    int bm_field_0;
    int nm_field_4;
    int nm_field_8;
    CPtrList nfield_C;
    int nm_field_28;
    int m_m_field_2C;
    int nm_field_30;
};

#endif /* CUNKNOWN2_H_ */
