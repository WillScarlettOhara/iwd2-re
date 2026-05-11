#ifndef CRESBINARY_H_
#define CRESBINARY_H_

#include "CRes.h"

class CResBinary : public CRes {
public:
    CResBinary();
    ~CResBinary();

    /* 0054 */ int m_nField54;
};

#endif /* CRESBINARY_H_ */
