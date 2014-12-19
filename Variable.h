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

#if !defined Variable_h
#define Variable_h

#include "globals.h"
#include <math.h>
#include <float.h>
#include <vector>
#include <malloc.h>
#include <string>


typedef struct {
	//CStringArray sCode;	// list of codes generated from recode, sorted upwards
        std::vector<std::string> sCode;
	int nCode;              // number of codes, should be equal to sCode.GetSize()
	//CString Missing1;       // first missing value
        std::string Missing1;   // first missing value
	//CString Missing2;       // second missing value
        std::string Missing2;       // second missing value
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
		//sCode.SetSize(0, 20);
		//Recode.sCode.SetSize(0, 20);
                Recode.DestCode = 0;

		Priority = 50;
		SetMissing = false;
		HasRecode = false;
		HasRound = false;
		HasCodingTop = false;
		HasCodingBottom = false;
		HasWeightNoise = false;
		HasPram = false;
		IsHHIdent = false;
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

	bool IsHHIdent;
	bool IsHHVar;
	bool IsCategorical;
	bool IsNumeric;
	bool IsWeight;

	int nMissing;
	std::string Missing1;
	std::string Missing2;
	bool SetMissing;

	
	double MinValue;
	double MaxValue;
	std::vector<std::string> sCode;
	int nCode;

	bool HasRecode;
	RECODE Recode;
	/// Some more stuff comes here
	int TableIndex;         // index in table (temporarely used during tabulation and MakeSafe)
	bool TableIsMissing;    // index is Missing
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
	std::string TopString;
  
	bool HasCodingBottom;  
	double BottomLevel;
	std::string BottomString;

	bool HasWeightNoise;
        double WeightNoise;

        bool HasPram;           // Variable has Post RAndomization Method
        //CUIntArray PramValue;   // for every code a percentage
        std::vector<unsigned int> PramValue;   // for every code a percentage
        long PramBandWidth;     // Number of codes (+ and -) in which code can be changed


	//functions methods

	bool   SetPosition(long lbPos, long lnPos, long lnDec);
	bool   SetType(bool bIsCategorical, bool bIsNumeric, bool bIsWeight, bool bIsHHIdent, bool bIsHHVar);
	void   SetMissingString(std::string sMissing1, std::string sMissing2);
	bool   AddCode(const char *newcode, bool tail);
	int    BinSearchStringArray(std::vector<std::string> &s, std::string x, int nMissing, bool &IsMissing);
	void   SortCodeLists();
	bool   SetTableIndex(std::string scode); 
	int    GetnCodes(bool WithMissing);
	void   UndoRecode();
	bool   PrepareRecode();
	int    AddRecode(const char *newcode);
	int    MakeRecodelistEqualWidth(std::string sMissing1, std::string sMissing2);
	int    FindCode(std::string code,int nMissing, bool &IsMissing);
	int    FindRecodedCode(std::string code, int nMissing, bool &isMissing);
	void   SortCode(int first, int last);
	void   SortRecodedCode(int first, int last);
	long   ComputeWidth(long nPos, long HHIdentOption, long nRecords);
	double DoRound(double val);
	double DoWeightNoise(double val);
	
private:
	void QuickSortStringArray(std::vector<std::string> &s, int first, int last);
	int  BinSearchStringArray(std::string x, bool& IsMissing);
	void AddSpacesBefore(std::string& str, int len);
};

#endif
