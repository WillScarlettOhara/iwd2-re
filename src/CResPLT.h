#ifndef CRESPLT_H_
#define CRESPLT_H_

#include "CRes.h"

class CResPLT : public CRes {
public:
    CResPLT();
    ~CResPLT();

    /* 0050 */ int nfield_50;
    /* 0054 */ int nfield_54;
    /* 0058 */ int nfield_58;
};

#endif /* CRESPLT_H_ */
