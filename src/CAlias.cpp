#include "CAlias.h"

// #not-binary-identical
// 0x781720
CAlias::CAlias(CString& a2)
{
    if (a2.GetLength() > 0) {
        ParseRawData(a2);
    }
}

// #binary-identical
// 0x7817B0
CAlias::~CAlias()
{
}

// #binary-identical
// 0x781800
BOOL CAlias::ParseRawData(CString& a2)
{
    CString temp;

    m_bParsed = FALSE;

    int pos = a2.FindOneOf(":=");
    if (pos > 0) {
        m_sAlias = a2.Left(pos + 1);

        m_sAlias += '\\';
        if (m_sAlias.GetLength() > 2 && a2.GetLength() > pos + 2) {
            m_bParsed = TRUE;

            a2 = a2.Right(a2.GetLength() - pos - 2);

            while (a2.GetLength() > 0) {
                int semicolonPos = a2.Find(';');
                if (semicolonPos == -1) {
                    temp = a2;
                    if (temp.GetAt(temp.GetLength() - 1) != '\\') {
                        temp += "\\";
                    }
                    m_lPaths.AddTail(temp);
                    a2 = "";
                } else {
                    temp = a2.Left(semicolonPos);

                    if (temp.GetAt(temp.GetLength() - 1) != '\\') {
                        temp += "\\";
                    }

                    m_lPaths.AddTail(temp);

                    a2 = a2.Right(a2.GetLength() - semicolonPos - 1);
                }
            }
        }
    }

    return m_bParsed;
}
