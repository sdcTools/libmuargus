# if ! defined Recode_h
# define Recode_h

class CRecode
{
public:
//	CStringArray sCode;	// list of codes generated from recode, sorted upwards
	int nCode;              // number of codes, should be equal to sCode.GetSize()
//	CString Missing1;       // first missing value
//	CString Missing2;       // second missing value
	int nMissing;	        // number of missing codes (usually 1 or 2)
        int *DestCode;          // m_var[SrcVar].nCodes times an index to sCode
	int CodeWidth;          // all sCodes have this width
};

#endif