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

#include "Table.h"
#include <assert.h>
#include <string.h>

bool CTable::SetVariables(long lnDim, long *ExplVar, long BirVarnr)
{
    nDim = lnDim;

    for (int i = 0; i < nDim; i++) {
	Varnr[i] = ExplVar[i] - 1;
    }

    BIRWeightVar = BirVarnr-1;
    return true;
}

bool CTable::CheckVarSequence()
{
    int i;
    unsigned int v = Varnr[0];

    for (i = 1; i < nDim; i++) {
        if (Varnr[i] <= (int) v) return false;
        v = Varnr[i];
    }

    return true;
}

long CTable::GetMemSize()
{
    long MemSizeTable,j;
    MemSizeTable = 1;
    for (j = 0; j < nDim; j++) {
        MemSizeTable *= SizeDim[j]; // number of codes of variable
    }
    return MemSizeTable;
}

bool CTable::PrepareTable()
{
    long MemSizeTable;
    MemSizeTable = GetMemSize();
    //Cell = (long *) malloc(MemSizeTable * sizeof(long) );
    Cell = new long [MemSizeTable];
    if (Cell == 0) {
        return false;
    }
    if (IsBIR) {
      //m_tab[i].BIRCell = (double *) malloc(MemSizeTable * sizeof(double) );
        BIRCell = new double [MemSizeTable];
	if (BIRCell == 0) {
            return false;
        }
    }

    nCell = MemSizeTable;  // for ASSERT

    // make all cells zero
    memset(Cell, 0, MemSizeTable * sizeof(long) );
    if (IsBIR) {
        memset(BIRCell, 0, MemSizeTable * sizeof(double) );
    }

    return true;
}

void CTable::FreeRecodedTable()
{
    if (Cell != 0) {
        delete[] Cell;
    }
    if (BIRCell != 0) {
	delete[] BIRCell;
    }
    Cell = 0;
    BIRCell = 0;
}

long CTable::GetCellNr(int *DimNr)
{ 
    long c = 0;

    for (int i = 0; i < nDim; i++) {
	assert(DimNr[i] >= 0 && DimNr[i] < SizeDim[i]);
	c *= SizeDim[i];
	c += DimNr[i];
    }

    return c;
}
/*
void CTable:: operator = (CTable & table2)
{
	bool tempreturn;
	int i;
	Threshold = table2.Threshold;  
	nDim= table2.nDim;              // number of variables (= dimensions) in table
	//SizeDim[MAXDIM];   // = nCode of corresponding variable
	//Varnr[MAXDIM];     // index of each dimension variable

	for (i=0; i<MAXDIM; i++) {
		SizeDim[i] = table2.SizeDim[i];
	}

	for (i=0; i<MAXDIM; i++) {
		Varnr[i] = table2.Varnr[i];
	}
	//long *Cell;             // counting space for frequency
	nCell = table2.nCell;             // number of cells
	
	BaseTable = table2.BaseTable;         // is a permanent table, specified bij SetTable?
	HasRecode= table2.HasRecode;         // toggle for Recode Table (index at i + m_ntab)
  
	IsBIR = table2.IsBIR;           // toggle for BaseIndividualRisk table
	//double *BIRCell;        // counting space for weight (BIR)
	BIRThreshold= table2.BIRThreshold;    // Threshold for BIR
	BIRWeightVar = table2.BIRWeightVar;    // index weight variable
	BIRMinValue  =table2.BIRMinValue;
	BIRMaxValue = table2.BIRMaxValue;
	BIRClassWidth = table2.BIRClassWidth ;
	BIRnClasses = table2.BIRnClasses;
	BIRUnsafe = table2.BIRUnsafe;

	if (table2.Cell != 0) {
		if (Cell != 0) {
        delete[] Cell;
      }
      if (BIRCell != 0) {
        delete[] BIRCell;
      }
		Cell = 0;
		BIRCell = 0;
		
		tempreturn = PrepareTable();

		for (i=0; i<nCell; i++) {
			Cell[i] = table2.Cell[i];
			if (IsBIR)
			{
				BIRCell[i] = table2.BIRCell[i];
			}
		}
	}
	
	
}
*/
