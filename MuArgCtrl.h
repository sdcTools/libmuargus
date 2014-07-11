// MuArgCtrl.h : Declaration of the CMuArgCtrl

#ifndef __MUARGCTRL_H_
#define __MUARGCTRL_H_

#include "Variable.h"
#include "Table.h"
#include "globals.h"
#include "ChSafeVarInfo.h"
#include "UCList.h"
#include "VarList.h"


#include <math.h>
#include <float.h>
//#include "MuArgCP.h"
#include "Household.h"
/*CString extern CurrentHHName;
CString extern LastHHName;
*/
/////////////////////////////////////////////////////////////////////////////
// CMuArgCtrl
class CMuArgCtrl
{
public:
	CMuArgCtrl()
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
	~CMuArgCtrl()
	{
		CleanUp();
	}

private:
	long m_nvar;
	CVariable *m_var;

	long m_ntab;
	CTable *m_tab;

	int m_HHIdentVar;
	//CUIntArray m_HHVars;
        std::vector<unsigned int> m_HHVars;
	long m_fSize;

	int m_fixedlength;         // computed fixed length record in micro data

	long m_nChangeFiles;
	CChSafeVarInfo * m_ChangeFiles;

	bool m_InFileIsFixedFormat;
	//CString m_InFileSeperator;
        std::string m_InFileSeperator;
	bool m_IgnoreFirstLine;
	//CString m_FirstLine;
        std::string m_FirstLine;
	long m_NumberofRecs;

	bool m_OutFileIsFixedFormat;
	//CString m_OutFileSeperator;
        std::string m_OutFileSeperator;
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

	//CString m_WarningRecode;
        std::string m_WarningRecode;

	int m_nvarpos;
	CVarList *m_varlist;        // Data positions from source to safe record

	bool m_WriteRandom;
	long m_HHIdentOption;
	int m_HHSeqNr;

	bool m_WithPriority;       // Set Missing in equal cases depending on Priority
	bool m_WithEntropy;        // Set Missing in equal cases depending on Entropy

	int m_WriteCoPrime;
	int m_WriteRecNr;
	int m_SafeRecordLength;
	int m_nUnsafe;

	//Household
	CHousehold *m_HH;
	long m_lNumberOfHH;
	std::string CurrentHHName ;
	std::string LastHHName;
	bool m_bHasHH;
	bool m_bHasBIR;
	long m_lNumBIRs;

	//functions
	void CleanUp();
	void CleanTables();
	void CleanVars();

	void FireUpdateProgress(short Perc);

	int  ReadMicroRecord(FILE *fd, char *str);
	bool DoMicroRecord(char *str, int *varindex);
	bool ReadVariableFreeFormat(char *Str, long VarIndex, std::string *VarCode);
	bool ConvertNumeric(char *code, double &d);
	void AddSpacesBefore(std::string& str, int len);
	void AddSpacesBefore(char *str, int len);

	bool ComputeTableIndex(char *str, CVariable *var, long Index);
	void FillTables(char *str);
	void AddTableCell(CTable& t, double Weight);
	int  ComputeSubTableList();
	void DoSubTableList(int iTab, int niv, int from, int *vars, int CVar);
	int  ComputeSubTable(CTable &BaseTable, CTable &SubTable);
	void MakeSubTable(CTable& BaseTab, CTable& SubTab, int niv, int iParentCell, int iSubCell, int *tabvars);
	bool ComputeUnsafeCells(CTable & t, int CVar);
	void ComputeNumberUnsafeCells(CTable & t, int niv, int cindex, bool IsMissingCode, int CVar = -1, int code = -1);
	void SortUCList(int n, CUCList *uc);
	void QuickSortUCList(CUCList *s, int first, int last);
	int  CompareUCListThres(CUCList& a, CUCList& b);
	int  CompareUCList(CUCList& a, CUCList& b);

	void SetTableHasRecode();

	bool ComputeRecodeTables();
	bool ComputeRecodeTable(CTable & srctab, CTable & dsttab);
	void ComputeRecodeTableCells(CTable & srctab, CTable & dsttab, int niv, int iCellSrc, int iCellDst);

	int  SetCode2Recode(int VarIndex, char *DestCode,char *SrcCode1, char *SrcCode2, int fromto);
	//bool ParseRecodeString(long VarIndex, std::string RecodeString, long FAR* ErrorType, long FAR* ErrorLine, long FAR* ErrorPos, int Phase);
        bool ParseRecodeString(long VarIndex, std::string RecodeString, long *ErrorType, long *ErrorLine, long *ErrorPos, int Phase);
	//bool ParseRecodeStringLine(long VarIndex, std::string str, long FAR* ErrorType, long FAR* ErrorPos, int Phase);
        bool ParseRecodeStringLine(long VarIndex, std::string str, long *ErrorType, long *ErrorPos, int Phase);
	int  ReadWord(std::string str, char* CodeFrom, char *CodeTo, char EndCode, int& fromto, int& pos);

	bool WriteVariablesFromMicroRecord(char *str, FILE *fdout, long *VarIndexes,long nVar, std::string seperator);

	bool AddMissingTable(CTable & t, int niv, int *DimNr, bool HasMissing, int type, double *Ksi, double MaxRisk, long * Frequency=0);
	int  AddMissing(const CTable& tab, int *DimNr, long& freq, double& weight, bool HasMissing);
	void AddMissingCells(CTable& t, int *dimnr, int *nMissing, long& freq, double& weight);

	bool MakeRecordDescription(long HHIdentOption);
	bool MakeFreeRecordDescription(long HHIdentOption);
	void QuickSortStringArray(std::vector<std::string> &s, int first, int last);
	bool DoEntropy(long VarNr, double& Entropy);
	bool MakeRecordSafe(char *record, int fase, int recnr, int nRecHH, long HHNum);
	bool ComputeVarIndices(char *record);
	int  ComputeRecordUC(long HHNum);
	void SetVarMissing(int iVar);
	double SetFreqMissings(std::vector<unsigned int>& FreqMissing, int nRecHH);
	int GetHHSizeFactor(int VarIndex, int nRecHH);
	double SetMinMissings(std::vector<unsigned int>& MinMissing, int nRecHH);
	bool WriteRecord(FILE *fd_out, char *record, long HHIdentOption, long recnr, bool WithBHR, long HHNum, bool PrintBIR, char *origrecord);
	bool IsInOutputFile(long VarIndex, long *FileNum, long *ArrIndex);
	int GetRandomInteger();
	int ComputeRecHH(FILE *fd, int bpos, int npos);
	bool DoCompleteHH(FILE *fd, FILE *fd_out, long StartPos, int n_rec, int *InvolvedVar, int recnr, long HHIdentOption, long HHNum, bool PrintBIR);
	void ComputeTableBIR(CTable &t, int& BIRFreq, double& BIRWeight, double& BIR);

	bool NumberOfHH(char *str, long &HHNumbers);
	bool IsNewHH(char *str);
	bool FindBIRForRec(char *str, double *BIRarray);
	void QuickSortDoubleArray(double *d, int first, int last);
	bool FillBIRArray(CTable &tab, double *BIRarray, char *record);
	void QuickSortBIRFreqArray(double * BIR, long *Freq, int first, int last);
	double FindBIRforNumIterations(double BIR0, long NumIter, long nUnsafe, double *BIRArray, long *FreqArray, CTable &t);



	int GGD(int a, int b);

public:
	bool ComputeBIRRateThreshold(long TableIndex, double Risk, double *ReIdentRate);
	long NumberOfHouseholds();
	bool CalculateBHRFreq(/*[in]*/ long TableIndex, /*[in]*/ bool UseNumOfHH, /*[in]*/ long nUnsafeHH, /*[in]*/ long nUnsafeRec, /*[in,out]*/ double * ResBHR, /*[in,out]*/  long * ErrCode);
	bool CalculateBIRFreq(/*[in]*/ long TableIndex,  /*[in]*/ long nUnsafe, /*[in,out]*/ double *BIRResult, /*[in,out]*/ long *ErrorCode);
	bool SetBHRThreshold(/*[in]*/ long TableIndex, /*[in]*/ double BHRThreshold, /*[in,out]*/ long *nUnsafeHH, /*[in,out]*/ long *nUnsafeRec);
	bool GetBHRHistogramData(/*[in]*/ long TableIndex, /*[in]*/ long nClasses, /*[in,out]*/ double * ClassLeftValue, /*[in,out]*/   long * HHFrequency, /* [in,out] */ long * RecFrequency);
	bool CalculateBaseHouseholdRisk(/*[in,out] */ long * ErrorCode);
	bool MakeFileSafe(/*[in]*/ std::string FileName,/*[in]*/  bool WithPrior, /*[in]*/ bool  WithEntropy, /*[in]*/ long HHIdentOption, /*[in]*/ bool RandomizeOutput,/*[in]*/ bool PrintBHR);
	bool GetBIRHistogramData(/*[in]*/ long TabIndex, /*[in]*/ long nClasses,/*[in,out]*/  double *ClassLeftValue, /*[in,out]*/ double *Ksi, /*[in,out]*/ long *Frequency);
	bool MakeFileSafeClearOptions();
	bool SetWeightNoise(/*[in]*/ long VarIndex, /*[in]*/ double WeightNoise, /*[in]*/ bool Undo);
	bool SetSuppressPrior(/*[in]*/ long VarIndex, /*[in]*/ long Priority);
	bool SetRound(/*[in]*/ long VarIndex, /*[in]*/ double RoundBase, /*[in]*/ long nDec, /*[in]*/ bool Undo);
	bool SetChangeFile(/*[in]*/ long FileIndex, /*[in]*/ std::string FileName, /*[in]*/ long nVar, /*[in,out]*/ long *VarIndex, /*[in]*/ std::string FileSeperator);
	bool GetVarProperties(/*[in]*/ long VarIndex, /*[in,out]*/ long *StartPos, /*[in,out]*/ long *nPos, /*[in,out]*/ long *nSuppress, /*[in,out]*/ double *Entropy, /*[in,out]*/ long *BandWidth,/*[in,out]*/  std::string *Missing1,/*[in,out]*/  std::string *Missing2, /*[in,out]*/ long *NofCodes, /*[in,out]*/ long *NofMissing);
	bool GetVarCode(/*[in]*/ long VarIndex, /*[in]*/ long CodeIndex, /*[in,out]*/ std::string *Code, /*[in,out]*/ long *PramPerc);
	bool GetTableUC(/*[in]*/ long nDim, /*[in]*/ long Index,/*[in,out]*/ bool *BaseTable,/*[in,out]*/ long *nUC, /*[in,out]*/ long *VarList);
	bool SetPramValue(/*[in]*/ long CodeIndex, /*[in]*/ long Value);
	bool SetPramVar(/*[in]*/ long VarIndex,/*[in]*/ long BandWidth,/*[in]*/ bool Undo);
	bool ClosePramVar(/*[in]*/ long VarIndex);
	bool GetMinMaxValue(/*[in]*/ long VarIndex,/*[in,out]*/  double *Min, /*[in,out]*/  double *Max);
	bool SetCodingBottom(/*[in]*/ long VarIndex,/*[in]*/ double BottomLevel,/*[in]*/ std::string BottomString,/*[in]*/ bool BottomUndo);
	bool SetCodingTop(/*[in]*/ long VarIndex, /*[in]*/ double TopLevel,/*[in]*/  std::string TopString,/*[in]*/  bool TopUndo);
	bool SetBirThreshold(/*[in]*/ long TabIndex, /*[in]*/ double Threshold, /*[in,out]*/ long *nUnsafe);
	bool SetNumberOfChangeFiles(/*[in]*/ long nFiles);
	bool CleanAll();
	bool SetOutFileInfo(/*[in]*/ bool IsFixedFormat,/*[in]*/ std::string Seperator,/*[in]*/ std::string FirstLine,/*[in]*/ bool StringsInQuotes);
	long NumberofRecords();
	bool SetInFileInfo(/*[in]*/ bool IsFixedFormat,/*[in]*/ std::string Seperator,/*[in]*/ bool IgnoreFirstLine);
	bool WriteVariablesInFile(/*[in]*/ std::string FileNameMicro, /*[in]*/ std::string FileNameOut, /*[in]*/ long nVar, /*[in,out]*/ long *VarIndexes, /*[in]*/ std::string seperator, /*[in,out]*/ long *ErrorCode);
	bool ApplyRecode();
	bool DoTruncate(/*[in]*/ long VarIndex, /*[in]*/ long nPos);
	bool UndoRecode(/*[in]*/ long VarIndex);
	bool DoRecode(/*[in]*/ long VarIndex,/*[in]*/ std::string RecodeString,/*[in]*/ std::string eMissing1,/*[in]*/ std::string eMissing2,/*[in,out]*/ long *ErrorType,/*[in,out]*/ long *ErrorLine, /*[in,out]*/ long *ErrorPos, /*[in,out]*/ std::string *WarningString);
	bool UnsafeVariableClose(/*[in]*/ long VarIndex);
	bool UnsafeVariableCodes(/*[in]*/ long VarIndex,/*[in]*/  long CodeIndex,/*[in,out]*/  long *IsMissing, /*[in,out]*/  long *Freq,/*[in,out]*/  const char **Code, /*[in,out]*/ long *Count, /*[in,out]*/ long *UCArray);
	bool UnsafeVariablePrepare(/*[in]*/ long VarIndex, /*[in]*/  long *nCode);
	bool UnsafeVariable(/*[in]*/ long VarIndex, /*[in,out]*/ long * Count, /*[in,out]*/ long * UCArray);
	long GetMaxnUC();
	bool ComputeTables(/*[in]*/ long *ErrorCode, /*[in]*/ long *TableIndex);
	bool BaseIndividualRisk(/*[in]*/ long fk,/*[in]*/  double Fk,/*[in,out]*/  double *risk);
	bool SetTable(/*[in]*/ long TabIndex, /*[in]*/ long Threshold, /*[in]*/ long nDim, /*[in,out]*/ long *VarList, /*[in]*/ bool IsBIR,long BIRWeightVarIndex);
	bool SetNumberTab(/*[in]*/ long nTab);
	bool ExploreFile(/*[in]*/ std::string FileName,/*[in]*/  long *ErrorCode,/*[in]*/  long *LineNumber, /*[in]*/ long *VarIndex);
	bool SetVariable(/*[in]*/ long Index,/*[in]*/ long bPos,/*[in]*/ long nPos,/*[in]*/ long nDec, /*[in]*/ std::string Missing1,/*[in]*/ std::string Missing2,/*[in]*/  bool IsHHIdent,/*[in]*/  bool IsHHVar,/*[in]*/  bool IsCategorical,/*[in]*/  bool IsNumeric,/*[in]*/  bool IsWeight,/*[in]*/ long RelatedVar);
	bool SetNumberVar(/*[in]*/ long nvar);
};

#endif //__NEWMUARGCTRL_H_
