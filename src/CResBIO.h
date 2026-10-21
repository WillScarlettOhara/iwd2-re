#ifndef CRESBIO_H_
#define CRESBIO_H_

#include "CRes.h"

class CResBIO : public CRes {
public:
    // Inlined in CBaldurChitin::AllocResObject.
    CResBIO() { m_bParsed = FALSE; }
    ~CResBIO() override;
    void* Demand();
    int Release();

    /* 0050 */ BOOL m_bParsed;
};

#endif /* CRESBIO_H_ */
