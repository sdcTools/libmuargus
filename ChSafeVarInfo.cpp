/*
* Argus Open Source
* Software to apply Statistical Disclosure Control techniques
* 
* Copyright 2014 Statistics Netherlands
* 
* This program is free software; you can redistribute it and/or 
* modify it under the terms of the European Union Public Licence 
* (EUPL) version 1.1, as published by the European Commission.
* 
* You can find the text of the EUPL v1.1 on
* https://joinup.ec.europa.eu/software/page/eupl/licence-eupl
* 
* This software is distributed on an "AS IS" basis without 
* warranties or conditions of any kind, either express or implied.
*/

#include "ChSafeVarInfo.h"
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>

void CChSafeVarInfo::SetVarIndex(long *lVarIndex) {
    if (m_lNVar >0){
	m_lVarIndex = new long[m_lNVar];
	memcpy(m_lVarIndex, lVarIndex, m_lNVar*sizeof(long));
    }
}

std::string CChSafeVarInfo::GetStringFromFile()
{
	FILE *fd;
	std::string cstr;
	char str[MAXRECORD];
	fd = fopen(m_sFileName.c_str(),"r");
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
	std::string stempstr,stemp;
	char str[MAXRECORD];
	long icount;
	int iseppos;
	// First Clear previous array
	sVariableCode.clear();
	sVariableCode.resize(m_lNVar);
	fd = fopen(m_sFileName.c_str(),"r");
	fseek(fd,m_lCurrFilePos,SEEK_SET);


	fgets((char *)str, MAXRECORD,fd); // fgets() returns a string including any LF, CR, CRLF, LFCR
        str[strcspn(str,"\r\n")]=0; // Remove LF, CR, CRLF, LFCR, ... Important to do so for non-numeric variables!!!!
        stempstr = str;
        
	iseppos = stempstr.find(m_sSeperator,0);
	icount = 0;
	while (iseppos != -1)  {
            //stemp=stempstr.Left(iseppos);
            stemp = stempstr.substr(0, iseppos);
            //sVariableCode.SetAt(icount,stemp);
            sVariableCode.at(icount) = stemp;
            icount ++;
            //stempstr.Delete (0,iseppos +1);
            stempstr.erase(0,iseppos +1);
            iseppos = stempstr.find(m_sSeperator,0);
            if (iseppos == stempstr.npos) iseppos = -1; // std::string.find(x) returns npos if x is not found
	}

	if ((stempstr.length() == 0) || (icount < m_lNVar-1)	|| (icount > m_lNVar-1))
	{
            return false;
	}
	else
	{
            sVariableCode.at(icount) = stempstr;
	}
	m_lCurrFilePos = ftell(fd);
	fclose(fd);
	return true;
}
