#ifndef CALIAS_H_
#define CALIAS_H_

#include "mfc.h"

class CAlias : public CObject {
public:
    CAlias(CString& a2);
    ~CAlias();

    BOOL ParseRawData(CString& a2);

    /* 0004 */ BOOL m_bParsed;
    /* 0008 */ CString m_sAliasPrefix;
    /* 000C */ CStringList m_lReplacementPaths;
};

#endif /* CALIAS_H_ */
