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

#if !defined ChSafeVarInfo_h
#define ChSafeVarInfo_h

#define MAXRECORD 32000

#include <string>
#include <vector>

class CChSafeVarInfo
{

protected:
	std::string m_sFileName;
	long m_lNVar;
	long m_lCurrFilePos;
	std::string m_sSeperator;


public:
	long *m_lVarIndex;
	std::vector<std::string> sVariableCode;
	std::string GetSeperator()  { return m_sSeperator;}
	std::string GetFileName()   { return m_sFileName;}
	long GetNumberVar()         { return m_lNVar;}
	long GetCurrFilePos()       { return m_lCurrFilePos;}
	

	void SetSeperator(std::string sSeperator) {this->m_sSeperator = sSeperator;}
	void SetFileName(std::string sfilename) { this->m_sFileName = sfilename;}
	void SetNumberVar(long lnvar)    { this->m_lNVar = lnvar;}
	void SetCurrFilePos(long lPos)   { this->m_lCurrFilePos =lPos;}
	void SetVarIndex(long *lVarIndex);
	bool FillVariableCode();

	std::string GetStringFromFile();
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
