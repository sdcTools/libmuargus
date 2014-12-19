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

#if ! defined UCList_h
#define UCList_h

#include "Table.h"

class CUCList 
{
public:
	CUCList()
	{
            long i;
            TabNr = 0;
            nDim = 0;
            for (i=0; i<MAXDIM; i++) {
            	Varnr[i] = 0;
            }
            Threshold = 0;
            nUC = 0;
            biggestThreshold = 0;
            unsafe = 0;
            HasPram = 0;
            //table =0;
	}
	~CUCList()
	{
	}

	int TabNr;              // permanent or recoded table from which subtable is computed
	int nDim;               // number of dimensions
	int Varnr[MAXDIM];      // variable indices 
	int Threshold;          // no comment
	int nUC;                // number of UnsafeCombinations
	int biggestThreshold;   // combination of variables with biggest threshold (true/false)
	CTable table;           // content of subtable

	bool unsafe;            // during MakeFileSave: combination of variables is unsafe?
        bool HasPram;           // ture: at least one of the varialbles of the table is prammed
};

#endif
