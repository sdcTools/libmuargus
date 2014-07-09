#if !defined ChSafeVarInfo_h
#define ChSafeVarInfo_h

#define MAXRECORD 32000

class CChSafeVarInfo
{

protected:
	CString m_sFileName;
	long m_lNVar;
	long m_lCurrFilePos;
	CString m_sSeperator;


public:
	long *m_lVarIndex;
	CStringArray sVariableCode;
	CString GetSeperator()  { return m_sSeperator;}
	CString GetFileName()	{ return m_sFileName;}
	long GetNumberVar()		{ return m_lNVar;}
	long GetCurrFilePos()	{ return m_lCurrFilePos;}
	

	void SetSeperator(CString sSeperator) {this->m_sSeperator = sSeperator;}
	void SetFileName(CString sfilename) { this->m_sFileName = sfilename;}
	void SetNumberVar(long lnvar)    { this->m_lNVar = lnvar;}
	void SetCurrFilePos(long lPos)   { this->m_lCurrFilePos =lPos;}
	void SetVarIndex(long *lVarIndex);
	bool FillVariableCode();

	CString GetStringFromFile();
	CChSafeVarInfo()
	{
		m_sFileName = "";
		m_lNVar = 0;
		m_lVarIndex = 0;
		m_lCurrFilePos = 0;

	}
	~CChSafeVarInfo()
	{
		if (m_lNVar > 0) {
			delete[] m_lVarIndex;
		}

	}

};


#endif