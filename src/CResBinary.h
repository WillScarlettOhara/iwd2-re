#ifndef CRESBINARY_H_
#define CRESBINARY_H_

#include "CRes.h"

#pragma pack(push, 2)
class CResBinary : public CRes {
public:
    CResBinary();
    ~CResBinary();

    void* Demand();
    int Release();

    /* 0x0050 */ char* m_resVersion;
    /* 0x0054 */ BOOL m_bParsed;
};
#pragma pack(pop)

#endif /* CRESBINARY_H_ */
