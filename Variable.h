#if !defined Variable_h
#define Variable_h

#include "globals.h"
#include <math.h>
#include <float.h>


typedef struct {
	CStringArray sCode;	// list of codes generated from recode, sorted upwards
	int nCode;              // number of codes, should be equal to sCode.GetSize()
	CString Missing1;       // first missing value
	CString Missing2;       // second missing value
	int nMissing;	        // number of missing codes (usually 1 or 2)
	int *DestCode;          // m_var[SrcVar].nCodes times an index to sCode
	int CodeWidth;          // all sCodes have this width

} RECODE;

class CVariable {

public:
	CVariable()
	{
		PositionSet = false;
		bPos = -1;
		nPos = -1;
		nDec = -1;
		sCode.SetSize(0, 20);
		Recode.sCode.SetSize(0, 20);
		Recode.DestCode = 0;

		Priority = 50;
		SetMissing = false;
		HasRecode = false;
		HasRound = false;
		HasCodingTop = false;
		HasCodingBottom = false;
		HasWeightNoise = false;
		HasPram = false;
		IsHHIdent = FALSE;
		Entropy = -1;
		PramBandWidth = -1;
		RelatedTo = -1;

	}
	~CVariable()
	{
		if (HasRecode) {
			UndoRecode();
		}
		else {
			if (Recode.DestCode != 0) {
				free(Recode.DestCode);
				Recode.DestCode = 0;
			}
		}

	}
	long bPos;
	long nPos;
	long nDec;
	bool PositionSet;

	BOOL IsHHIdent;
	BOOL IsHHVar;
	BOOL IsCategorical;
	BOOL IsNumeric;
	BOOL IsWeight;

	int nMissing;
	CString Missing1;
	CString Missing2;
	bool SetMissing;

	
	double MinValue;
	double MaxValue;
	CStringArray sCode;
	int nCode;

	bool HasRecode;
	RECODE Recode;
	/// Some more stuff comes here
	int TableIndex;         // index in table (temporarely used during tabulation and MakeSafe)
	BOOL TableIsMissing;    // index is Missing
        int freq;               // frequency in unsafe tables
        int nSuppress;          // counts suppresses

	int Priority;           // to make the choice easier by set missings, value 0 - 100?
        double Entropy;         // to make the choice easier by set missings

        bool HasRound;          // do or do not rounding
        double RoundBase;       // Base for rounding
        int RoundnDec;          // number of decimals after rounding
        int RelatedTo;          // Var number it is related too (will be suppressed too)  
        bool HasCodingTop;      
	double TopLevel;
	CString TopString;
  
	bool HasCodingBottom;  
	double BottomLevel;
	CString BottomString;

	bool HasWeightNoise;
        double WeightNoise;

        bool HasPram;           // Variable has Post RAndomization Method
        CUIntArray PramValue;   // for every code a percentage
        long PramBandWidth;     // Number of codes (+ and -) in which code can be changed


	//functions methods

	bool  SetPosition(long lbPos, long lnPos, long lnDec);
	bool  SetType(bool bIsCategorical, bool bIsNumeric, bool bIsWeight, bool bIsHHIdent, bool bIsHHVar);
	void  SetMissingString(CString sMissing1, CString sMissing2);
	bool  AddCode(const char *newcode, bool tail);
	int   BinSearchStringArray(CStringArray &s, CString x, int nMissing, bool &IsMissing);
	void  SortCodeLists();
	bool  SetTableIndex(CString scode); 
	int   GetnCodes(BOOL WithMissing);
	void  UndoRecode();
	bool  PrepareRecode();
	int	AddRecode(const char *newcode);
	int   MakeRecodelistEqualWidth(LPCTSTR sMissing1, LPCTSTR sMissing2);
	int   FindCode(CString code,int nMissing, bool &IsMissing);
	int	FindRecodedCode(CString code, int nMissing, bool &isMissing);
	void	SortCode(int first, int last);
	void  SortRecodedCode(int first, int last);
	long	ComputeWidth(long nPos, long HHIdentOption, long nRecords);
	double DoRound(double val);
	double DoWeightNoise(double val);
	
private:
	void QuickSortStringArray(CStringArray &s, int first, int last);
	int  BinSearchStringArray(CString x, bool& IsMissing);
	void AddSpacesBefore(CString& str, int len);
};

#endif