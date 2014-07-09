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