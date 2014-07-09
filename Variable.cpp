#include "Variable.h"

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


void CVariable::SetMissingString(CString sMissing1, CString sMissing2)
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

	if (sMissing1.IsEmpty()) {
      Missing1 = sMissing2; 
      Missing2 = sMissing1;
    } 
	 else {
      Missing1 = sMissing1;
      Missing2 = sMissing2;
    }
    // second empty or equal?
	 if (sMissing2.IsEmpty() || sMissing1 == sMissing2) { 
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
  
  n = sCode.GetSize();

	// add at the end of array if not found
  if (tail) {  
    // not already in list?
    for (i = 0; i < n; i++) {
      if (sCode.GetAt(i) == newcode) break;  // found!
    }
    if (i == n) {  // not found
      sCode.Add(newcode);
    }
    return true;
  }

  // keep list sorted
  if (n == 0) {
    sCode.Add(newcode);
  } else {
    int res = BinSearchStringArray(sCode, newcode, 0, IsMissing);
    if (res < 0) { // not found
      for (i = 0; i < n; i++) {
        if (newcode < sCode.GetAt(i) ) break;
      }
      if (i < n) {
        sCode.InsertAt(i, newcode);
      } else {
        sCode.Add(newcode);
      }
    }
  }

  return true;
}

int CVariable::BinSearchStringArray(CStringArray &s, CString x, int nMissing, bool &IsMissing)
{ int mid, left = 0, right = s.GetSize() - 1 - nMissing;
  int mis;

  ASSERT(left <= right);

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
    mis = s.GetSize() - nMissing;
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

void CVariable::QuickSortStringArray(CStringArray &s, int first, int last)
{ int i, j;
  CString mid, temp; 

  ASSERT(first >= 0 && last >= first);
  
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

bool CVariable::SetTableIndex(CString scode)
{
	bool IsMissing;
	TableIndex = BinSearchStringArray(scode, IsMissing);
	TableIsMissing = IsMissing;

  // in case of a recode, take the recode index
  if (HasRecode) {
   TableIndex = Recode.DestCode[TableIndex];
    ASSERT(TableIndex >= 0);
    if (TableIndex < 0) {
      return false;   // program error
    }
  }

	return true;
}

void CVariable::AddSpacesBefore(CString& str, int len)
{ int width = str.GetLength();

  if (width >= len) return;  // nothing to do

  { char tempstr[100];
    sprintf(tempstr, "%*s", len - width, " ");
    str.Insert(0, tempstr);
  }
}

int CVariable::BinSearchStringArray(CString x, bool& IsMissing)
{ CStringArray s;
	int mid, mis;
  int left = 0;
	int right = sCode.GetSize() - 1 - nMissing;
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
    } else {
      if (x > sCode[mid]) {
        left = mid;
      } else {
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
    mis = sCode.GetSize() - nMissing;
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

int CVariable::GetnCodes(BOOL WithMissing)
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
	Recode.sCode.RemoveAll();
	Recode.nCode = 0;
	Recode.CodeWidth = 0;
	Recode.nMissing = 0;
	return true;

}

int CVariable::AddRecode(const char *newcode)
{
	int i, n;

	n = Recode.sCode.GetSize();
	// not already in list?
	for (i = 0; i < n; i++) {                               // optimizable with binary search (number > ??)
		if (Recode.sCode[i] == newcode) break;  // found!
	}
	if (i == n) {  // not found
		Recode.sCode.Add(newcode);
	}

	return i;
}

int CVariable::MakeRecodelistEqualWidth(LPCTSTR sMissing1, LPCTSTR sMissing2)
{ 
	int i, length, ncode = Recode.sCode.GetSize(), maxwidth = 0;

	// if missings empty take missings of variable[VarIndex]
	if (sMissing1[0] == 0 && sMissing2[0] == 0) {
		length = strlen(Missing1);
		if (length > maxwidth) maxwidth = length;
		length = strlen(Missing2);
		if (length > maxwidth) maxwidth = length;
	}

	for (i = 0; i < ncode; i++) {
		if (length = Recode.sCode[i].GetLength(), length > maxwidth) {
			maxwidth = length;
		}
	}
	if (strlen(sMissing1) > (UINT) maxwidth) maxwidth = strlen(sMissing1);
	if (strlen(sMissing2) > (UINT) maxwidth) maxwidth = strlen(sMissing2);
	Recode.CodeWidth = maxwidth;

	// spaces before shorter ones
	for (i = 0; i < ncode; i++) {
		AddSpacesBefore(Recode.sCode[i], maxwidth);
	}

	return maxwidth;
}

int CVariable::FindCode(CString code, int nMissing, bool &IsMissing)
{
	return BinSearchStringArray(sCode, code, nMissing, IsMissing);
}

int CVariable::FindRecodedCode(CString code, int nMissing, bool &IsMissing)
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
				np = __max(maxpos, minpos);
			}
        
			if (nMissing > 0) {
				np = __max(np, Missing1.GetLength() );
				np = __max(np, Missing2.GetLength() );
			}

			ASSERT(np > 0);
			// rounding
			if (HasRound) {
				n = sprintf(str, "%.*f", RoundnDec, DoRound(MaxValue) );
				np = n;
				n = sprintf(str, "%.*f", RoundnDec, DoRound(MinValue) );
				np = __max(np, n);
			}

			// top and bottom coding
			if (HasCodingTop) {
				n = TopString.GetLength();
				np = __max(np, n);
			}
			if (HasCodingBottom) {
				n = BottomString.GetLength();
				np = __max(np, n);
			}

			// Noise
			if (HasWeightNoise) {
				n = sprintf(str, "%.*f", nDec, MaxValue * (1 + WeightNoise) );
				np = __max(np, n);
				n = sprintf(str, "%.*f", nDec, MinValue * (1 + WeightNoise) );
				np = __max(np, n);
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

