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
