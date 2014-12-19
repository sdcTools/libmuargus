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

#include "Variable.h"
#include <assert.h>
#include <stdio.h>
#include <algorithm>

bool CVariable::SetPosition(long lbPos, long lnPos, long lnDec)
{
    bPos = lbPos - 1;
    nPos = lnPos;
    nDec = lnDec;
    PositionSet = true;
    return true;
}


bool CVariable::SetType(bool bIsCategorical, bool bIsNumeric, bool bIsWeight, bool bIsHHIdent, bool bIsHHVar)
{
    IsCategorical = bIsCategorical;
    IsNumeric = bIsNumeric;
    IsWeight = bIsWeight;
    IsHHIdent = bIsHHIdent;
    IsHHVar = bIsHHVar;
    if (IsWeight) {
    	IsNumeric = true;
	nMissing = 0;
	Missing1 = "";
	Missing2 = "";
    }
    return true;
}


void CVariable::SetMissingString(std::string sMissing1, std::string sMissing2)
{
	/*Missing1 = sMissing1;
	Missing2 = sMissing2;

	if (Missing1.IsEmpty()) {
		Missing1 = Missing2;
	}

	if (Missing1.IsEmpty() ) {
		nMissing = 0;
	} else {
		if (Missing2.IsEmpty() ) nMissing = 1;
		else                     nMissing = 2;
  }
  */
    
    //if (sMissing1.IsEmpty()) {
    if (sMissing1.empty()) {
      Missing1 = sMissing2; 
      Missing2 = sMissing1;
    } 
     else {
      Missing1 = sMissing1;
      Missing2 = sMissing2;
    }
    // second empty or equal?
    //if (sMissing2.IsEmpty() || sMissing1 == sMissing2) { 
    if (sMissing2.empty() || sMissing1 == sMissing2) { 
      Missing2 = sMissing1;
      nMissing = 1;
    } 
     else {
      nMissing = 2;
    }
  
}


bool CVariable::AddCode(const char *newcode, bool tail)
{ int i, n;
  bool IsMissing;
  std::vector<std::string>::iterator it;
  
  //n = sCode.GetSize();
  n = sCode.size();

  // add at the end of array if not found
  if (tail) {  
    // not already in list?
    for (i = 0; i < n; i++) {
      //if (sCode.GetAt(i) == newcode) break;  // found!
      if (sCode.at(i) == newcode) break;  // found!
    }
    if (i == n) {  // not found
      //sCode.Add(newcode);
      sCode.push_back(newcode);
    }
    return true;
  }

  // keep list sorted
  it = sCode.begin();
  if (n == 0) {
    //sCode.Add(newcode);
      sCode.push_back(newcode);
  } else {
    int res = BinSearchStringArray(sCode, newcode, 0, IsMissing);
    if (res < 0) { // not found
      for (i = 0; i < n; i++) {
        //if (newcode < sCode.GetAt(i) ) break;
          if (newcode < sCode.at(i) ) break;
      }
      if (i < n) {
        //sCode.InsertAt(i, newcode);
        sCode.insert(it+i,newcode);
      } else {
        //sCode.Add(newcode);
        sCode.push_back(newcode);
      }
    }
  }
  return true;
}

int CVariable::BinSearchStringArray(std::vector<std::string> &s, std::string x, int nMissing, bool &IsMissing)
{ int mid, left = 0, right = s.size() - 1 - nMissing;
  int mis;

  assert(left <= right);
  
  IsMissing = false;

  while (right - left > 1) {
    mid = (left + right) / 2;
    if (x < s[mid]) {
      right = mid;
    } else {
      if (x > s[mid]) {
        left = mid;
      } else {
        return mid;
      }
    }
  }

  if (x == s[right]) return right;
  if (x == s[left]) return left;

  // equal to missing1 or -2? // code missing not always the highest

  if (nMissing > 0) {
    mis = s.size() - nMissing;
    if (x == s[mis]) {
      IsMissing = true;
      return mis;
    }
    if (nMissing == 2) {
      if (x == s[mis + 1]) {
        IsMissing = true;
        return mis + 1;
      }
    }
  }

  return -1;
}

void CVariable::SortCodeLists()
{ 

    if (IsCategorical) {

      // always keep missing at the end of the list
		if (nCode - 1 - nMissing>0)
		{
		 QuickSortStringArray(sCode, 0, nCode - 1 - nMissing);
		}
	}

}

void CVariable::QuickSortStringArray(std::vector<std::string> &s, int first, int last)
{ int i, j;
  std::string mid, temp; 

  assert(first >= 0 && last >= first);
  
  do {
    i = first;
    j = last;
    mid = s[(i + j) / 2];
    do {
      while (s[i] < mid) i++;
      while (s[j] > mid) j--;
      if (i < j) {
        temp = s[i];
        s[i] = s[j];
        s[j] = temp;
      } else {
        if (i == j) {
          i++;
          j--;
        }
        break;
      }
    } while (++i <= --j);

    if (j - first < last - i) {
      if (j > first) {
        QuickSortStringArray(s, first, j);
      }
      first = i;
    } else {
      if (i < last) {
        QuickSortStringArray(s, i, last);
      }
      last = j;
    }
  } while (first < last);
} 

bool CVariable::SetTableIndex(std::string scode)
{
    bool IsMissing;
    TableIndex = BinSearchStringArray(scode, IsMissing);
    TableIsMissing = IsMissing;

    // in case of a recode, take the recode index
    if (HasRecode) {
        TableIndex = Recode.DestCode[TableIndex];
        assert(TableIndex >= 0);
        if (TableIndex < 0) {
            return false;   // program error
        }
    }

    return true;
}

void CVariable::AddSpacesBefore(std::string& str, int len)
{ int width = str.length();

  if (width >= len) return;  // nothing to do

  { //char tempstr[100];
    std::string tempstr;
    //sprintf(tempstr, "%*s", len - width, " ");
    tempstr.clear();
    tempstr.append(len - width,' ');
    //str.Insert(0, tempstr);
    str.insert(0,tempstr);
  }
}

int CVariable::BinSearchStringArray(std::string x, bool& IsMissing)
{   std::vector<std::string> s;
    int mid, mis;
    int left = 0;
    int right = sCode.size() - 1 - nMissing;
    //if right == 0 then 

  //ASSERT(left <= right);
	//if (left > right) {
	//	return -1;
	//}
   
    IsMissing = false;

  // do binary search
    while (right - left > 1) {
        mid = (left + right) / 2;
        if (x < sCode[mid]) {
            right = mid;
        } 
        else {
            if (x > sCode[mid]) {
                left = mid;
            } 
            else {
                return mid;
            }
        }
    }
    if (right>= 0) 
    {

        if (x == sCode[right]) return right;
        if (x == sCode[left]) return left;
    }

    else
    {
        return left;
    }

  // equal to missing1 or -2? // code missing not always the highest

    if (nMissing > 0) {
        mis = sCode.size() - nMissing;
        if (x == sCode[mis]) {
            IsMissing = true;
            return mis;
        }
        if (nMissing == 2) {
            if (x == sCode[mis + 1]) {
                IsMissing = true;
                return mis + 1;
            }
        }
    }

    return -1;  // code not found
}

int CVariable::GetnCodes(bool WithMissing)
{ 
    int n;
    if (HasRecode) {
        n = Recode.nCode;
        if (!WithMissing) n -= Recode.nMissing;
    } 
    else {
	n = nCode;
	if (!WithMissing) n -= nMissing;
    }

    return n;
}
void CVariable::UndoRecode()
{
	if (HasRecode) {
		if (Recode.DestCode != 0) {
			free(Recode.DestCode);
			Recode.DestCode = 0;
		}
	}
	HasRecode = false;
}

bool CVariable::PrepareRecode()
{
	int i;
	Recode.DestCode = (int *) malloc(nCode * sizeof(int) );
	if (Recode.DestCode == 0) {
		return false;
	}

	for (i = 0; i < nCode; i++) {
		Recode.DestCode[i] = -1;
	}	
	//Recode.sCode.RemoveAll();
        Recode.sCode.clear();
	Recode.nCode = 0;
	Recode.CodeWidth = 0;
	Recode.nMissing = 0;
	return true;

}

int CVariable::AddRecode(const char *newcode)
{
	int i, n;

	n = Recode.sCode.size();
	// not already in list?
	for (i = 0; i < n; i++) {                       // optimizable with binary search (number > ??)
		if (Recode.sCode[i] == newcode) break;  // found!
	}
	if (i == n) {  // not found
		Recode.sCode.push_back(newcode);
	}

	return i;
}

//int CVariable::MakeRecodelistEqualWidth(LPCTSTR sMissing1, LPCTSTR sMissing2)
int CVariable::MakeRecodelistEqualWidth(std::string sMissing1, std::string sMissing2)
{ 
	int i, length, ncode = Recode.sCode.size(), maxwidth = 0;

	// if missings empty take missings of variable[VarIndex]
	if (sMissing1[0] == 0 && sMissing2[0] == 0) {
		//length = strlen(Missing1);
                length = Missing1.length();
		if (length > maxwidth) maxwidth = length;
		//length = strlen(Missing2);
                length = Missing2.length();
		if (length > maxwidth) maxwidth = length;
	}

	for (i = 0; i < ncode; i++) {
		if (length = Recode.sCode[i].length(), length > maxwidth) {
			maxwidth = length;
		}
	}
	//if (strlen(sMissing1) > (UINT) maxwidth) maxwidth = strlen(sMissing1);
        if (sMissing1.length() > (unsigned int) maxwidth) maxwidth = sMissing1.length();
	//if (strlen(sMissing2) > (UINT) maxwidth) maxwidth = strlen(sMissing2);
        if (sMissing2.length() > (unsigned int) maxwidth) maxwidth = sMissing2.length();
	Recode.CodeWidth = maxwidth;

	// add spaces before shorter ones
	for (i = 0; i < ncode; i++) {
		AddSpacesBefore(Recode.sCode[i], maxwidth);
	}

	return maxwidth;
}

int CVariable::FindCode(std::string code, int nMissing, bool &IsMissing)
{
	return BinSearchStringArray(sCode, code, nMissing, IsMissing);
}

int CVariable::FindRecodedCode(std::string code, int nMissing, bool &IsMissing)
{
	return BinSearchStringArray(Recode.sCode, code, nMissing, IsMissing);
}

void CVariable::SortCode(int first, int last)
{
	QuickSortStringArray(sCode, first,last);
}

void CVariable::SortRecodedCode(int first, int last)
{
	QuickSortStringArray(Recode.sCode,first,last);
}

long CVariable::ComputeWidth(long nPos, long HHIdentOption, long nRecords)
{
    int np, n;
    np = nPos; // in most cases oke (?)
    if (HasRecode) {
        np = Recode.CodeWidth;
    } 
    else {
	if (IsNumeric) { // treat all possible numeric actions
            char str[MAXCODEWIDTH];
            np = 0;
            // source field
            //ASSERT(v->MaxValue != -DBL_MAX);
            //ASSERT(v->MinValue != DBL_MAX);

            if ((MaxValue == -DBL_MAX)|| (MinValue == DBL_MAX))
            {
		np = 0;
            }
            else
            {
				int maxpos = sprintf(str,"%.*f", nDec, MaxValue);
				int minpos = sprintf(str,"%.*f", nDec, MinValue);
				//np = __max(maxpos, minpos);
                                np = std::max(maxpos, minpos);
			}
        
			if (nMissing > 0) {
				np = std::max(np, (int) Missing1.length() );
				np = std::max(np, (int) Missing2.length() );
			}

			assert(np > 0);
			// rounding
			if (HasRound) {
				n = sprintf(str, "%.*f", RoundnDec, DoRound(MaxValue) );
				np = n;
				n = sprintf(str, "%.*f", RoundnDec, DoRound(MinValue) );
				np = std::max(np, n);
			}

			// top and bottom coding
			if (HasCodingTop) {
				n = TopString.length();
				np = std::max(np, n);
			}
			if (HasCodingBottom) {
				n = BottomString.length();
				np = std::max(np, n);
			}

			// Noise
			if (HasWeightNoise) {
				n = sprintf(str, "%.*f", nDec, MaxValue * (1 + WeightNoise) );
				np = std::max(np, n);
				n = sprintf(str, "%.*f", nDec, MinValue * (1 + WeightNoise) );
				np = std::max(np, n);
			}

			// adjust length strings top bottom
			if (HasCodingTop) {
				AddSpacesBefore(TopString, np);
			}
			if (HasCodingBottom) {
				AddSpacesBefore(BottomString, np);
			}
		}
   }
	// Make a boolean for HHIdentVar
   //if (VarIndex == m_HHIdentVar) {
	if (IsHHIdent)	{ // Is This not enought
		switch (HHIdentOption) {
			case HHIDENT_NO:   // shouldn't be possible
				break;
			case HHIDENT_KEEP:  // nothing to do;
				break;
			case HHIDENT_CHANGESEQNO:
			{ 
				char str[100];
				np = sprintf(str, "%d", nRecords);
			}
			break;
			case HHIDENT_DELETE:
				np = 0;
				break;
		}
   }
 
  return np;
}

// rounding a double
double CVariable::DoRound(double val)
{ double i;

  if (val < 0) {
    modf((val - RoundBase / 2 - .0000000001) / RoundBase, &i);
  } else {
    modf((val + RoundBase / 2 + .0000000001) / RoundBase, &i);
  }

  return i * RoundBase;

}


double CVariable::DoWeightNoise(double val)
{ 
	int i, v;
	double k;

	v = (int) (WeightNoise * 200);   // max v = 200.0 * 100 = 20.000

	i = rand() % v - v / 2;  // value rand() 0 - 32767
	k = (double) i / 10000.0;

	return val * (1 + k);

}

