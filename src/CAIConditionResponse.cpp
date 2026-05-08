#include "CAIConditionResponse.h"

#include "CAIUtil.h"

// NOTE: Inlined.
CAIConditionResponse::CAIConditionResponse()
{
}

// 0x40F960
CAIConditionResponse::CAIConditionResponse(const CAICondition& condition, const CAIResponseSet& responseSet)
{
    m_condition.Set(condition);
    m_responseSet.Set(responseSet);
}

// 0x40F9E0
CAIConditionResponse::~CAIConditionResponse()
{
}

// 0x40EA70
void CAIConditionResponse::Read(CString sData)
{
    CString sConditionData;
    CString sResponseSetData;

    sConditionData = CAIUtil::ReadBetween(sData, CString("CO\n"));
    m_condition.Read(sConditionData);

    sResponseSetData = CAIUtil::ReadBetween(sData, CString("RS\n"));
    m_responseSet.Read(sResponseSetData);
}

// Phase 1-2: Scaffold functions
// 0x40F940
void FUN_0040f940() {
    // TODO: Incomplete.
}

