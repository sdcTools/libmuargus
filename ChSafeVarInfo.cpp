#include "ChSafeVarInfo.h"

void CChSafeVarInfo::SetVarIndex(long *lVarIndex) {
	if (m_lNVar >0){
		m_lVarIndex = new long[m_lNVar];
		m_lVarIndex = lVarIndex;

	}
}

CString CChSafeVarInfo::GetStringFromFile()
{
	FILE *fd;
	CString cstr;
	char str[MAXRECORD];
	fd = fopen(m_sFileName,"r");
	fseek(fd,m_lCurrFilePos,SEEK_SET);
	fgets(str,MAXRECORD,fd);
	cstr = str;
	m_lCurrFilePos= ftell(fd);
	fclose(fd);
	return cstr;
}

bool CChSafeVarInfo::FillVariableCode()
{
	FILE *fd;
	CString stempstr,stemp;
	UCHAR str[MAXRECORD];
	long icount;
	int iseppos;
	// First Clear previos array
	sVariableCode.RemoveAll();
	sVariableCode.SetSize(m_lNVar);
	fd = fopen(m_sFileName,"r");
	fseek(fd,m_lCurrFilePos,SEEK_SET);


	fgets((char *)str, MAXRECORD,fd);
	stempstr = str;
	iseppos = stempstr.Find(m_sSeperator,0);
	icount = 0;
	while (iseppos != -1)  {
		stemp=stempstr.Left(iseppos);
		sVariableCode.SetAt(icount,stemp);
		icount ++;
		stempstr.Delete (0,iseppos +1);
		iseppos = stempstr.Find(m_sSeperator,0);

	}

	if ((stempstr.GetLength() == 0) || (icount < m_lNVar-1)
			|| (icount > m_lNVar-1))
	{
		return false;

	}
	else
	{
		sVariableCode.SetAt(icount,stempstr);
	}
	m_lCurrFilePos = ftell(fd);
	fclose(fd);
	return true;
}