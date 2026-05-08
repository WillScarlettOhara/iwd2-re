#ifndef CALIAS_H_
#define CALIAS_H_

#include "mfc.h"

class CAlias : public CObject {
public:
    CAlias(CString& a2);
    ~CAlias();

    BOOL ParseRawData(CString& a2);

    /* 0004 */ BOOL bm_field_4;
    /* 0008 */ CString sm_field_8;
    /* 000C */ CStringList pfield_C;
};

#endif /* CALIAS_H_ */
