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

#if !defined Table_h
#define Table_h

#include "globals.h"

class CTable {
public:
	CTable()
	{
		Cell = 0;
		nCell = 0;
		HasRecode = false;
		IsBIR = false;
		BIRCell = 0;
		BIRThreshold =0;
		BHRThreshold = 0;
	}
	~CTable()
	{
		/*if (Cell != 0) {
        delete[] Cell;
      }
      if (BIRCell != 0) {
        delete[] BIRCell;
      }
		Cell = 0;
		BIRCell = 0;
		*/
	}
	long Threshold;  
	long nDim;              // number of variables (= dimensions) in table
	int  SizeDim[MAXDIM];   // = nCode of corresponding variable
	int  Varnr[MAXDIM];     // index of each dimension variable
	long *Cell;             // counting space for frequency
	long nCell;             // number of cells
	
	bool BaseTable;         // is a permanent table, specified bij SetTable?
	bool HasRecode;         // toggle for Recode Table (index at i + m_ntab)
  
	bool   IsBIR;           // toggle for BaseIndividualRisk table
	double *BIRCell;        // counting space for weight (BIR)
	double BIRThreshold;    // Threshold for BIR
	long   BIRWeightVar;    // index weight variable
	double BIRMinValue;
	double BIRMaxValue;
	double BIRClassWidth;
	double BIRnClasses;
	long   BIRUnsafe;
	// Methods and Functions

	// For Household Risk management
	double BHRMinValue;
	double BHRMaxValue;
	double BHRClassWidth;
	double BHRnClasses;
	long   BHRUnsafe;
	double BHRThreshold;

	bool SetVariables(long lnDim, long *ExplVar, long BirVarnr);
	bool CheckVarSequence();
	long GetMemSize();
	bool PrepareTable();
	void FreeRecodedTable();
	long GetCellNr(int *DimNr);
	//void operator = (CTable & table2);

};

#endif
