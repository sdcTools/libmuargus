// NewMuArgCtrl.h : Declaration of the CNewMuArgCtrl

#ifndef __NEWMUARGCTRL_H_
#define __NEWMUARGCTRL_H_

#include "resource.h"       // main symbols
#include "Variable.h"
#include "Table.h"
#include "globals.h"
#include "ChSafeVarInfo.h"
#include "UCList.h"
#include "VarList.h"


#include <math.h>
#include <float.h>
#include "NewMuArgCP.h"
#include "GlobalVar.h"
#include "Household.h"
/*CString extern CurrentHHName;
CString extern LastHHName;
*/
/////////////////////////////////////////////////////////////////////////////
// CNewMuArgCtrl
class ATL_NO_VTABLE CNewMuArgCtrl :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CNewMuArgCtrl, &CLSID_NewMuArgCtrl>,
	public IConnectionPointContainerImpl<CNewMuArgCtrl>,
	public IDispatchImpl<INewMuArgCtrl, &IID_INewMuArgCtrl, &LIBID_NEWMUARGLib>,
	public CProxy_INewMuArgCtrlEvents< CNewMuArgCtrl >
{
public:
	CNewMuArgCtrl()
	{
		m_nvar = 0;
		m_var = 0;
		m_ntab = 0;
		m_tab = 0;

		m_fname[0] = 0;

		m_UCList = 0;

		m_nUC = 0;
		m_unsafe = 0;
		m_varlist = 0;
		m_PramVarIndex = -1;
		m_WriteRandom = false;

		m_nChangeFiles = 0;

		m_InFileIsFixedFormat = true;
		m_OutFileIsFixedFormat = true;
		m_IgnoreFirstLine = false;
		m_FirstLine = "";
		m_NumberofRecs = -1;
		m_StringsInQuotes = false;

		m_HH = 0;
		CurrentHHName = "";
		LastHHName = "";
		m_lNumberOfHH = 0;
		m_bHasHH = false;
		m_bHasBIR = false;
		m_lNumBIRs = 0;
	}
	~CNewMuArgCtrl()
	{


		CleanUp();
	}

private:


	long m_nvar;
	CVariable *m_var;

	long m_ntab;
	CTable *m_tab;

	int m_HHIdentVar;
	CUIntArray m_HHVars;
	long m_fSize;

	int m_fixedlength;         // computed fixed length record in micro data

	long m_nChangeFiles;
	CChSafeVarInfo * m_ChangeFiles;

	bool m_InFileIsFixedFormat;
	CString m_InFileSeperator;
	bool m_IgnoreFirstLine;
	CString m_FirstLine;
	long m_NumberofRecs;

	bool m_OutFileIsFixedFormat;
	CString m_OutFileSeperator;
	bool m_StringsInQuotes;

	int m_nRecFile;
	char m_fname[_MAX_PATH];

	int m_PramVarIndex;


	CUCList *m_UCList;

	long m_cuc;
	long m_nUC;

	int (*m_unsafe)[MAXDIM + 1];  // first one for freq, others dim 1, 2, ...

	int m_maxdim;


	int m_nOverlap;
	int m_nUntouched;
	int m_nNoSense;

	CString m_WarningRecode;

	int m_nvarpos;
	CVarList *m_varlist;        // Data positions from source to safe record

	BOOL m_WriteRandom;
	long m_HHIdentOption;
	int m_HHSeqNr;

	BOOL m_WithPriority;       // Set Missing in equal cases depending on Priority
	BOOL m_WithEntropy;        // Set Missing in equal cases depending on Entropy

	int m_WriteCoPrime;
	int m_WriteRecNr;
	int m_SafeRecordLength;
	int m_nUnsafe;

	//Household
	CHousehold *m_HH;
	long m_lNumberOfHH;
	CString CurrentHHName ;
	CString LastHHName;
	bool m_bHasHH;
	bool m_bHasBIR;
	long m_lNumBIRs;

	//functions
	void CleanUp();
	void CleanTables();
	void CleanVars();

	void FireUpdateProgress(short Perc);

	int ReadMicroRecord(FILE *fd, UCHAR *str);
	BOOL DoMicroRecord(UCHAR *str, int *varindex);
	bool ReadVariableFreeFormat(UCHAR *Str, long VarIndex, CString *VarCode);
	bool ConvertNumeric(char *code, double &d);
	void AddSpacesBefore(CString& str, int len);
	void AddSpacesBefore(char *str, int len);

	BOOL ComputeTableIndex(UCHAR *str, CVariable *var, long Index);
	void FillTables(UCHAR *str);
	void AddTableCell(CTable& t, double Weight);
	int  ComputeSubTableList();
	void DoSubTableList(int iTab, int niv, int from, int *vars, int CVar);
	int  ComputeSubTable(CTable &BaseTable, CTable &SubTable);
	void MakeSubTable(CTable& BaseTab, CTable& SubTab, int niv, int iParentCell, int iSubCell, int *tabvars);
	BOOL ComputeUnsafeCells(CTable & t, int CVar);
	void ComputeNumberUnsafeCells(CTable & t, int niv, int cindex, BOOL IsMissingCode, int CVar = -1, int code = -1);
	void SortUCList(int n, CUCList *uc);
	void QuickSortUCList(CUCList *s, int first, int last);
	int CompareUCListThres(CUCList& a, CUCList& b);
	int CompareUCList(CUCList& a, CUCList& b);

	void SetTableHasRecode();

	BOOL ComputeRecodeTables();
	BOOL ComputeRecodeTable(CTable & srctab, CTable & dsttab);
	void ComputeRecodeTableCells(CTable & srctab, CTable & dsttab, int niv, int iCellSrc, int iCellDst);

	int  SetCode2Recode(int VarIndex, char *DestCode,char *SrcCode1, char *SrcCode2, int fromto);
	BOOL ParseRecodeString(long VarIndex, LPCTSTR RecodeString, long FAR* ErrorType, long FAR* ErrorLine, long FAR* ErrorPos, int Phase);
	BOOL ParseRecodeStringLine(long VarIndex, LPCTSTR str, long FAR* ErrorType, long FAR* ErrorPos, int Phase);
	int  ReadWord(LPCTSTR str, char* CodeFrom, char *CodeTo, char EndCode, int& fromto, int& pos);

	BOOL WriteVariablesFromMicroRecord(UCHAR *str, FILE *fdout, long *VarIndexes,long nVar, CString seperator);

	BOOL AddMissingTable(CTable & t, int niv, int *DimNr, BOOL HasMissing, int type, double *Ksi, double MaxRisk, long * Frequency=0);
	int	AddMissing(const CTable& tab, int *DimNr, long& freq, double& weight, BOOL HasMissing);
	void AddMissingCells(CTable& t, int *dimnr, int *nMissing, long& freq, double& weight);

	bool MakeRecordDescription(long HHIdentOption);
	bool MakeFreeRecordDescription(long HHIdentOption);
	void QuickSortStringArray(CStringArray &s, int first, int last);
	BOOL DoEntropy(long VarNr, double& Entropy);
	BOOL MakeRecordSafe(UCHAR *record, int fase, int recnr, int nRecHH, long HHNum);
	BOOL ComputeVarIndices(UCHAR *record);
	int  ComputeRecordUC(long HHNum);
	void SetVarMissing(int iVar);
	double SetFreqMissings(CUIntArray& FreqMissing, int nRecHH);
	int GetHHSizeFactor(int VarIndex, int nRecHH);
	double SetMinMissings(CUIntArray& MinMissing, int nRecHH);
	BOOL WriteRecord(FILE *fd_out, UCHAR *record, long HHIdentOption, long recnr, bool WithBHR, long HHNum, bool PrintBIR, UCHAR *origrecord);
	bool IsInOutputFile(long VarIndex, long *FileNum, long *ArrIndex);
	int GetRandomInteger();
	int ComputeRecHH(FILE *fd, int bpos, int npos);
	BOOL DoCompleteHH(FILE *fd, FILE *fd_out, long StartPos, int n_rec, int *InvolvedVar, int recnr, long HHIdentOption, long HHNum, bool PrintBIR);
	void ComputeTableBIR(CTable &t, int& BIRFreq, double& BIRWeight, double& BIR);

	bool NumberOfHH(UCHAR *str, long &HHNumbers);
	bool IsNewHH(UCHAR *str);
	bool FindBIRForRec( UCHAR *str, double *BIRarray);
	void QuickSortDoubleArray(double *d, int first, int last);
	bool FillBIRArray(CTable &tab, double *BIRarray, UCHAR *record);
	void QuickSortBIRFreqArray(double * BIR, long *Freq, int first, int last);
	double FindBIRforNumIterations(double BIR0, long NumIter, long nUnsafe, double *BIRArray, long *FreqArray, CTable &t);



	int GGD(int a, int b);
public:
DECLARE_REGISTRY_RESOURCEID(IDR_NEWMUARGCTRL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CNewMuArgCtrl)
	COM_INTERFACE_ENTRY(INewMuArgCtrl)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CNewMuArgCtrl)
CONNECTION_POINT_ENTRY(DIID__INewMuArgCtrlEvents)
END_CONNECTION_POINT_MAP()


// INewMuArgCtrl
public:
	STDMETHOD(ComputeBIRRateThreshold)(long TableIndex, double Risk, double *ReIdentRate, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(NumberOfHouseholds)(/*[retval,out]*/ long *pVal);
	STDMETHOD(CalculateBHRFreq)(/*[in]*/ long TableIndex, /*[in]*/ VARIANT_BOOL UseNumOfHH, /*[in]*/ long nUnsafeHH, /*[in]*/ long nUnsafeRec, /*[in,out]*/ double * ResBHR, /*[in,out]*/  long * ErrCode, /*[retval,out]*/  VARIANT_BOOL *pVal );
	STDMETHOD(CalculateBIRFreq)(/*[in]*/ long TableIndex,  /*[in]*/ long nUnsafe, /*[in,out]*/ double *BIRResult, /*[in,out]*/ long *ErrorCode, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetBHRThreshold)(/*[in]*/ long TableIndex, /*[in]*/ double BHRThreshold, /*[in,out]*/ long *nUnsafeHH, /*[in,out]*/ long *nUnsafeRec, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetBHRHistogramData)(/*[in]*/ long TableIndex, /*[in]*/ long nClasses, /*[in,out]*/ double * ClassLeftValue, /*[in,out]*/   long * HHFrequency, /* [in,out] */ long * RecFrequency, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(CalculateBaseHouseholdRisk)(/*[in,out] */ long * ErrorCode,/*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(MakeFileSafe)(/*[in]*/ BSTR FileName,/*[in]*/  VARIANT_BOOL WithPrior, /*[in]*/ VARIANT_BOOL  WithEntropy, /*[in]*/ long HHIdentOption, /*[in]*/ VARIANT_BOOL RandomizeOutput,/*[in]*/ VARIANT_BOOL PrintBHR, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetBIRHistogramData)(/*[in]*/ long TabIndex, /*[in]*/ long nClasses,/*[in,out]*/  double *ClassLeftValue, /*[in,out]*/ double *Ksi, /*[in,out]*/ long *Frequency, /*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(MakeFileSafeClearOptions)();
	STDMETHOD(SetWeightNoise)(/*[in]*/ long VarIndex, /*[in]*/ double WeightNoise, /*[in]*/ VARIANT_BOOL Undo,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(SetSuppressPrior)(/*[in]*/ long VarIndex, /*[in]*/ long Priority,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(SetRound)(/*[in]*/ long VarIndex, /*[in]*/ double RoundBase, /*[in]*/ long nDec, /*[in]*/ VARIANT_BOOL Undo, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetChangeFile)(/*[in]*/ long FileIndex, /*[in]*/ BSTR FileName, /*[in]*/ long nVar, /*[in,out]*/ long *VarIndex, /*[in]*/ BSTR FileSeperator, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetVarProperties)(/*[in]*/ long VarIndex, /*[in,out]*/ long *StartPos, /*[in,out]*/ long *nPos, /*[in,out]*/ long *nSuppress, /*[in,out]*/ double *Entropy, /*[in,out]*/ long *BandWidth,/*[in,out]*/  BSTR *Missing1,/*[in,out]*/  BSTR *Missing2, /*[in,out]*/ long *NofCodes, /*[in,out]*/ long *NofMissing, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetVarCode)(/*[in]*/ long VarIndex, /*[in]*/ long CodeIndex, /*[in,out]*/ BSTR *Code, /*[in,out]*/ long *PramPerc, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetTableUC)(/*[in]*/ long nDim, /*[in]*/ long Index,/*[in,out]*/ VARIANT_BOOL *BaseTable,/*[in,out]*/ long *nUC, /*[in,out]*/ long *VarList, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetPramValue)(/*[in]*/ long CodeIndex, /*[in]*/ long Value,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(SetPramVar)(/*[in]*/ long VarIndex,/*[in]*/ long BandWidth,/*[in]*/ VARIANT_BOOL Undo,/*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(ClosePramVar)(/*[in]*/ long VarIndex, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetMinMaxValue)(/*[in]*/ long VarIndex,/*[in,out]*/  double *Min, /*[in,out]*/  double *Max, /*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(SetCodingBottom)(/*[in]*/ long VarIndex,/*[in]*/ double BottomLevel,/*[in]*/ BSTR BottomString,/*[in]*/ VARIANT_BOOL BottomUndo, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetCodingTop)(/*[in]*/ long VarIndex, /*[in]*/ double TopLevel,/*[in]*/  BSTR TopString,/*[in]*/  VARIANT_BOOL TopUndo,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(SetBirThreshold)(/*[in]*/ long TabIndex, /*[in]*/ double Threshold, /*[in,out]*/ long *nUnsafe,/*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetNumberOfChangeFiles)(/*[in]*/ long nFiles,/*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(CleanAll)();
	STDMETHOD(SetOutFileInfo)(/*[in]*/ VARIANT_BOOL IsFixedFormat,/*[in]*/ BSTR Seperator,/*[in]*/ BSTR FirstLine,/*[in]*/ VARIANT_BOOL StringsInQuotes);
	STDMETHOD(NumberofRecords)(/*[retval,out]*/ long *pVal);
	STDMETHOD(SetInFileInfo)(/*[in]*/ VARIANT_BOOL IsFixedFormat,/*[in]*/ BSTR Seperator,/*[in]*/ VARIANT_BOOL IgnoreFirstLine);
	STDMETHOD(WriteVariablesInFile)(/*[in]*/ BSTR FileNameMicro, /*[in]*/ BSTR FileNameOut, /*[in]*/ long nVar, /*[in,out]*/ long *VarIndexes, /*[in]*/ BSTR seperator, /*[in,out]*/ long *ErrorCode, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(ApplyRecode)();
	STDMETHOD(DoTruncate)(/*[in]*/ long VarIndex, /*[in]*/ long nPos,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(UndoRecode)(/*[in]*/ long VarIndex,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(DoRecode)(/*[in]*/ long VarIndex,/*[in]*/ BSTR RecodeString,/*[in]*/ BSTR eMissing1,/*[in]*/ BSTR eMissing2,/*[in,out]*/ long *ErrorType,/*[in,out]*/ long *ErrorLine, /*[in,out]*/ long *ErrorPos, /*[in,out]*/ BSTR *WarningString,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(UnsafeVariableClose)(/*[in]*/ long VarIndex,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(UnsafeVariableCodes)(/*[in]*/ long VarIndex,/*[in]*/  long CodeIndex,/*[in,out]*/  long *IsMissing, /*[in,out]*/  long *Freq,/*[in,out]*/  BSTR *Code, /*[in,out]*/ long *Count, /*[in,out]*/ long *UCArray, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(UnsafeVariablePrepare)(/*[in]*/ long VarIndex, /*[in]*/  long *nCode, /*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(UnsafeVariable)(/*[in]*/ long VarIndex, /*[in,out]*/ long * Count, /*[in,out]*/ long * UCArray, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetMaxnUC)(/*[retval,out]*/ long *pVal);
	STDMETHOD(ComputeTables)(/*[in]*/ long *ErrorCode, /*[in]*/ long *TableIndex, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(BaseIndividualRisk)(/*[in]*/ long fk,/*[in]*/  double Fk,/*[in,out]*/  double *risk, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetTable)(/*[in]*/ long TabIndex, /*[in]*/ long Threshold, /*[in]*/ long nDim, /*[in,out]*/ long *VarList, /*[in]*/ VARIANT_BOOL IsBIR,long BIRWeightVarIndex, /*[retval,out]*/ VARIANT_BOOL *pVal);
	STDMETHOD(SetNumberTab)(/*[in]*/ long nTab,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(ExploreFile)(/*[in]*/ BSTR FileName,/*[in]*/  long *ErrorCode,/*[in]*/  long *LineNumber, /*[in]*/ long *VarIndex,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(SetVariable)(/*[in]*/ long Index,/*[in]*/ long bPos,/*[in]*/ long nPos,/*[in]*/ long nDec, /*[in]*/ BSTR Missing1,/*[in]*/ BSTR Missing2,/*[in]*/  VARIANT_BOOL IsHHIdent,/*[in]*/  VARIANT_BOOL IsHHVar,/*[in]*/  VARIANT_BOOL IsCategorical,/*[in]*/  VARIANT_BOOL IsNumeric,/*[in]*/  VARIANT_BOOL IsWeight,/*[in]*/ long RelatedVar,/*[retval,out]*/  VARIANT_BOOL *pVal);
	STDMETHOD(SetNumberVar)(/*[in]*/ long nvar, /*[retval,out]*/ VARIANT_BOOL *pVal);
};

#endif //__NEWMUARGCTRL_H_
