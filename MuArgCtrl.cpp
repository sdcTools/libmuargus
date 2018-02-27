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

// MuArgCtrl.cpp : Implementation of CMuArgCtrl


#include "MuArgCtrl.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

void WriteLog(std::string String){
    FILE *f_log = fopen("C:\\Users\\pwof\\AppData\\Local\\Temp\\AnonLog.txt","a");
    fprintf(f_log,"%s\n",String.c_str());
    fclose(f_log);
}

std::string trimright(const std::string &t)
{
    std::string str = t;
    size_t found;
    found = str.find_last_not_of(" \n\r\t");
    if (found != str.npos)
    	str.erase(found+1);
    else
    	str.clear();            // str is all whitespace

    return str;
}

std::string trimleft(const std::string &t)
{
    std::string str = t;
    size_t found;
    found = str.find_first_not_of(" \n\r\t");
    if (found != str.npos)
    	str.erase(0,found);
    else
    	str.clear();            // str is all whitespace

    return str;
}

/////////////////////////////////////////////////////////////////////////////
// CMuArgCtrl

void CMuArgCtrl::SetProgressListener(IProgressListener* ProgressListener)
{
    m_ProgressListener = ProgressListener;
}

void CMuArgCtrl::FireUpdateProgress(int Perc)
{
	if (m_ProgressListener != NULL) {
		m_ProgressListener->UpdateProgress(Perc);
	}
}

/**
 * Specifies the number of variables, reserves memory for the administration of the variables
 * @param nvar number of variables
 * returns false if nvar is not correct (<1) or insufficient memory
 */
bool CMuArgCtrl::SetNumberVar(long nvar)
{
//	CleanUp();  // in case of a second call very usefull
    m_nvar = nvar;
    m_var = new CVariable[m_nvar];
    if (m_var == 0) {
        return false;
    }
    m_ntab = 0;
    m_tab = 0;

    if (m_nvar <= 0)  {
        return false;
    }
    else  {
        return true;
    }
}

/**
 * Specifies the properties of a variable
 * @param Index         1,2,... index of variable
 * @param bPos          1,2,... starting position in micro data file
 * @param nPos          1,2,... number of positions in micro data file, up to 100
 * @param nDec          0,1,... number of decimal places (especially important when writing safe file
 * @param Missing1      Code for first missing 
 * @param Missing2      Code for second missing
 * @param IsHHIdent     Is identification variable for household
 * @param IsHHVar       Is a household variable
 * @param IsCategorical Is a categorical variable
 * @param IsNumeric     Is a numeric variable
 * @param IsWeight      Is a weight
 * @param RelatedVar    Index of related variable
 * @return              false if one or more parameters is wrong
 */
bool CMuArgCtrl::SetVariable(long Index, long bPos, long nPos, long nDec, std::string Missing1, std::string Missing2,
				bool IsHHIdent, bool IsHHVar, bool IsCategorical, bool IsNumeric, bool IsWeight, long RelatedVar)
{
    long i = Index - 1;
    std::string sMissing1 = Missing1;
    std::string sMissing2 = Missing2;
    bool bIsHHIdent,bIsHHVar,bIsCategorical,bIsNumeric,bIsWeight;

    if (IsHHIdent) {
	bIsHHIdent = true;
    }
    else {
	bIsHHIdent = false;
    }

    if (IsHHVar) {
	bIsHHVar = true;
    }
    else {
	bIsHHVar = false;
    }

    if (IsCategorical) {
	bIsCategorical = true;
    }
    else {
	bIsCategorical = false;
    }
    if (IsNumeric) {
	bIsNumeric = true;
    }
    else {
	bIsNumeric = false;
    }

    if (IsWeight) {
	bIsWeight = true;
    }
    else {
	bIsWeight = false;
    }


    // Not the right moment, first call SetNumberVar
    if (m_nvar == 0) {
        return false;
    }

    if (Index < 1 || Index > m_nvar || bPos < 1 || nDec < 0 || nPos < 1 || nPos >= MAXCODEWIDTH) {
        return false;
    }
    m_var[i].SetPosition(bPos,nPos,nDec);

    if (IsCategorical || (IsNumeric && !IsWeight) ) {
        // missings both empty?
        if (Missing1[0] == 0 && Missing2[0] == 0) {
            return false;
	}
	m_var[i].SetMissingString(sMissing1,sMissing2);
    }

    m_var[i].SetType(bIsCategorical,bIsNumeric,bIsWeight,bIsHHIdent,bIsHHVar);
    if (IsHHIdent) {
	m_HHIdentVar = i;
	m_bHasHH = true;
    }
    if (IsHHVar) {
	m_HHVars.push_back(i);
    }
    m_var[i].RelatedTo = RelatedVar - 1;

    return true;
}

/**
 * Examines of each categorical variable which codes occur. In fixed format input files,
 * all records are of equal length. The only exception are empty records, which are ignored 
 * without warning
 * @param FileName      Name of file to be investigated
 * @param ErrorCode     
 *                      FILENOTFOUND    file can not be opened
 *                      EMPTYFILE       file is empty
 *                      WRONGLENGTH     not all record lengths are equal
 *                      RECORDTOOSHORT  a variable does not fit within specified record length
 *                      NOVARIABLES     there are no variables specified 
 * @param LineNumber    Line number where error occurred
 * @param VarIndex      Index of variable where error occurred
 * @return              false in case of error
 */
bool CMuArgCtrl::ExploreFile(std::string FileName, long *ErrorCode, long *LineNumber, long *VarIndex)
{
    long tempNumberofHH = 0;
    std::string sFileName;
    sFileName = FileName;
    FILE *fd;
    char str[MAXRECORDLENGTH];
    int i, length, recnr = 0, varindex;

    *ErrorCode = *LineNumber = *VarIndex = 0;

    if (m_nvar == 0) {
	*ErrorCode = NOVARIABLES;
	return false;
    }

    fd = fopen(sFileName.c_str(), "r");
    if (fd == 0) {
        *ErrorCode = FILENOTFOUND;
        return false;
    }

    fseek(fd, 0, SEEK_END);
    m_fSize = ftell(fd);

    rewind(fd);
  // read first record to determine the fixed recordlength
    str[0] = 0;
    fgets((char *)str, MAXRECORDLENGTH, fd);
    if (str[0] == 0) {
	*ErrorCode = EMPTYFILE;
	goto error;
    }

    length = strlen((char *)str) - 1;
    
    while (length > 0 && str[length] < ' ') length--;
    m_fixedlength = length + 1;
    if (length == 0) {
	*ErrorCode = EMPTYFILE; // first record empty
	goto error;
    }

  // record length oke?
    for (i = 0; i < m_nvar; i++) {
	if (m_InFileIsFixedFormat) {
            if (m_var[i].bPos + m_var[i].nPos > m_fixedlength) {
		*ErrorCode = RECORDTOOSHORT;
		goto error;
            }
	}
    // initialize Min and Max Value for Numerics
        if (m_var[i].IsNumeric) {
            m_var[i].MaxValue = -DBL_MAX;
            m_var[i].MinValue = DBL_MAX;
        }
    }

    int res;
    rewind(fd);
    if ((!m_InFileIsFixedFormat)&&(m_IgnoreFirstLine)) {
        res = ReadMicroRecord(fd, str);
    }

    while (!feof(fd) ) {
        res = ReadMicroRecord(fd, str);
        switch (res) {
            case INFILE_ERROR:
                recnr++;
                *ErrorCode = WRONGLENGTH;
                *LineNumber = recnr;
                goto error;
            case  INFILE_EOF:
                goto oke;
            case  INFILE_OKE:
                recnr++;
//		if ((!m_InFileIsFixedFormat)&&(m_IgnoreFirstLine)&&(recnr == 1)) {
//			continue;
//		}
//		else {
		if (recnr % FIREPROGRESS == 0) {
                    FireUpdateProgress((int)(ftell(fd) * 100.0 / m_fSize));  // for progressbar in container
		}
		if (m_bHasHH) {
                    if (!NumberOfHH(str, tempNumberofHH) )	{
                        goto error;
                    }
		}
		if (!DoMicroRecord(str, &varindex) ) {
                    *ErrorCode = WRONGRECORD;
                    *LineNumber = recnr;
                    *VarIndex = varindex;
                     goto error;
		}
                break;
        }
    }

    oke:
    fclose(fd);
    m_nRecFile = recnr;
    m_NumberofRecs = recnr;
    if (m_bHasHH){
        m_lNumberOfHH = tempNumberofHH +1; // for the last household
        // terug zetten
        CurrentHHName =  "";
        LastHHName = "";
    }
    for (i = 0; i < m_nvar; i++) {
        if (m_var[i].IsCategorical) {
            m_var[i].AddCode(m_var[i].Missing1.c_str(), true);
            m_var[i].AddCode(m_var[i].Missing2.c_str(), true);
        }

        m_var[i].nCode = m_var[i].sCode.size();  // save for later use
    }
    FireUpdateProgress(100);  // for progressbar in container
    strcpy(m_fname, sFileName.c_str());

    for (i = 0; i < m_nvar; i++) {
	m_var[i].SortCodeLists();
    }
    return true;
    
    error:
    fclose(fd);
    return false;
}

int CMuArgCtrl::ReadMicroRecord(FILE *fd, char *str)
{
    int length = 0;

    while (length == 0) {
        str[0] = 0;
        fgets(str, MAXRECORDLENGTH, fd);
        if (str[0] == 0) return INFILE_EOF;
        length = strlen(str) - 1;
        while (length > 0 && str[length] < ' ') length--;
        if (length == 0) continue;  // skip empty records
        str[length + 1] = 0;
        if (m_InFileIsFixedFormat) {
            if (length  + 1 != m_fixedlength) {
		return INFILE_ERROR;
            }
        }
    }
    return INFILE_OKE;
}

bool CMuArgCtrl::DoMicroRecord(char *str, int *varindex)
{ 
    int i, bp, ap;
    char code[MAXCODEWIDTH];
    CVariable *var;
    std::string tempcode;

    for (i = 0; i < m_nvar; i++) {
	*varindex = i + 1;
	var = &(m_var[i]);
	if (var->IsCategorical || var->IsNumeric) {
            if(m_InFileIsFixedFormat) {
                bp = var->bPos;         // startposition
                ap = var->nPos;         // number of positions
                strncpy(code, (const char *)&str[bp], ap); // get code from record
                code[ap] = 0;
            }
            else {
                ap = var->nPos;         // number of positions
		if (ReadVariableFreeFormat(str,i,&(tempcode))) {
                    strcpy(code,(const char*)tempcode.c_str());
                    code[ap] = 0;
		}
            }
        }
	else {
            continue;
        }

        // Only add if not Missing code!!!!! ANCO
        if (var->IsCategorical) { // only a categorical var has a codelist
            if ((code != var->Missing1) && (code != var->Missing2))
            {
		//if (var->AddCode(i, code, false) ) {   // adds if new, else does nothing
                if (!(var->AddCode(code,false))){
                    return false;
		}
            }
        }

        if (var->IsNumeric) {
            double d;
      // exclude missings for calculating min/max
            if (strcmp(code, var->Missing1.c_str()) != 0 && strcmp(code, var->Missing2.c_str()) != 0 ) {
                if (!ConvertNumeric(code, d) ) return false;   // is not numeric!

                if (d > var->MaxValue) var->MaxValue = d;
                if (d < var->MinValue) var->MinValue = d;
            }
        }
    }

    return true;
}

bool CMuArgCtrl::ReadVariableFreeFormat(char *Str, long VarIndex, std::string *VarCode)
{
    std::vector<std::string> VarCodes;
    std::string stempstr, stemp, tempvarcode;
    CVariable *var;
    VarCodes.resize(m_nvar);
    stempstr = Str;
    int inrem;
    long lseppos;
    long lcount= 0;
    if (m_InFileSeperator  != " ") {
	lseppos = stempstr.find(m_InFileSeperator,0);
	while (lseppos != -1) {
            stemp = stempstr.substr(0,lseppos);
            VarCodes.at(lcount) = stemp;
            stempstr.erase(0, lseppos + 1);
            lcount ++;
            lseppos = stempstr.find(m_InFileSeperator, 0);
            if (lseppos == stempstr.npos) lseppos = -1; // std::string.find returns npos if nothing found
	}
	//fill the stuff here
	if ((stempstr.length() == 0) || (lcount <m_nvar - 1) || (lcount >= m_nvar)) {
            // string too short or too long
            return false;
	}
	else {
            VarCodes.at(lcount) = stempstr;
	}
	tempvarcode = VarCodes.at(VarIndex);
	//tempvarcode.TrimLeft();
        tempvarcode = trimleft(tempvarcode);
	//tempvarcode.TrimRight();
        tempvarcode = trimright(tempvarcode);
	//inrem = tempvarcode.Remove('"');
        inrem = tempvarcode.size();
        tempvarcode.erase(std::remove(tempvarcode.begin(),tempvarcode.end(),'"'),tempvarcode.end());
        inrem = inrem - tempvarcode.size(); // Number of removed quotes
	assert ((inrem == 2) || (inrem == 0));  // should be either 2 or 0
	var = &(m_var[VarIndex]);
	AddSpacesBefore(tempvarcode,var->nPos);
	//Now add leading spaces
	*VarCode = tempvarcode;
	return true;
    }
    else {
        return false;
    }
}

bool CMuArgCtrl::ConvertNumeric(char *code, double &d)
{ 
    char *stop;

    //Setting the locale to C is not necessary for Windows, but it seems to be for Unix
    std::string s = std::string(setlocale(LC_NUMERIC, "C"));
    d = strtod(code, &stop);

    if (*stop != 0) {
        while (*(stop) == ' ') stop++;
        if (*stop != 0) {
        return false;
        }
    }
    return true;
}

void CMuArgCtrl::AddSpacesBefore(std::string& str, int len)
{ 
    int width = str.length();

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

void CMuArgCtrl::AddSpacesBefore(char *str, int len)
{ int lstr = strlen(str);

  if (lstr >= len) return;  // nothing to do

  char tempstr[MAXCODEWIDTH + 1];
  strcpy(tempstr, str);
  sprintf(str, "%*s%s", len - lstr, " ", tempstr);
}


/**
 * Specifies the number of the table to compute.
 * Clears previously specified tables, if any.
 * @param nTab  Number of tables
 * @return      false if nTab is incorrect or if insufficient memory
 */
bool CMuArgCtrl::SetNumberTab(long nTab)
{
    if (m_nvar == 0 || nTab < 1) {
        return false;
    }

    // ev. free and delete previous tables
    if (m_ntab != 0) {
        CleanTables();
    }

    m_ntab = nTab;
    m_tab = new CTable[nTab + nTab];  // second part for recoded tables
    if (m_tab == 0) {
        return false;
    }
    return true;
}

/**
 * Deletes all specified data and displays all the reserved memory. 
 * Also called by SetNumberVar
 * @return always true
 */
bool CMuArgCtrl::CleanAll()
{
    CleanUp();
    return true;
}

void CMuArgCtrl::CleanUp()
{
    for (int i = 0; i < m_nUC; i++) {
	if (m_UCList[i].nDim != m_tab[m_UCList[i].TabNr].nDim) {  // no base table
            delete[] m_UCList[i].table.Cell;
            m_UCList[i].table.Cell = 0;
            m_UCList[i].table.nCell = 0;
            if (m_UCList[i].table.IsBIR) {
                if (m_UCList[i].table.BIRCell != 0) {
                    delete[] m_UCList[i].table.BIRCell;
                    m_UCList[i].table.BIRCell = 0;
		}
            }
	}
    }
    // variables
    CleanVars();
    
    // tables
    CleanTables();
    
    if (m_HH != 0) {
        delete[] m_HH;
    }
    m_HH = 0;
    
    // List Unsafe Combinations
    if (m_UCList != 0) {
        delete [] m_UCList;
	m_UCList = 0;
    }

    // number unsafe combinations
    m_nUC = 0;

    // name datafile
    m_fname[0] = 0;

    if (m_unsafe != 0) {
        delete [] m_unsafe;
	m_unsafe = 0;
    }

    if (m_varlist != 0) {
	delete [] m_varlist;
	m_varlist = 0;
    }

    m_HHIdentVar = -1;    // make HHIdent na
    m_HHVars.clear();     // remove all HHVars
    if (m_nChangeFiles > 0) {
        delete[] m_ChangeFiles;
        m_nChangeFiles = 0;
    }

    m_lNumBIRs = 0;
    m_lNumberOfHH = 0;
}

void CMuArgCtrl::CleanVars()
{
    if (m_nvar > 0) {
    /*for (i = 0; i < m_nvar; i++) {
      if (m_var[i].Recode.DestCode != 0) {
        free(m_var[i].Recode.DestCode);
      }
    }*/
	delete [] m_var;
    }

    m_var = 0;
    m_nvar = 0;
}

void CMuArgCtrl::CleanTables()
{
    // also free the used Cells from the tables
    if (m_ntab != 0) {
        for (int i = 0; i < m_ntab + m_ntab; i++) {
            if (m_tab[i].Cell != 0) {
                delete[] m_tab[i].Cell;
            }
            if (m_tab[i].BIRCell != 0) {
                delete[] m_tab[i].BIRCell;
            }
        }
        delete [] m_tab;
    }

    m_tab = 0;
    m_ntab = 0;
}

/**
 * Specifies the attributes of a table.
 * The threshold is not important in case IsBIR == true.
 * In case of BIR, the threshold is specified after inspection of the histogram.
 * @param TabIndex          Index of the table
 * @param Threshold         Threshold
 * @param nDim              Number of dimensions (<=10)
 * @param VarList           The index for each dimension of the variable (1, 2, ..., nVar)
 * @param IsBIR             Is individual-risk-base table
 * @param BIRWeightVarIndex Index of variabe containing BIR-associated weight
 * @return false in case of specification errors
 */
bool CMuArgCtrl::SetTable(long TabIndex, long Threshold, long nDim, long *VarList, bool IsBIR, long BIRWeightVarIndex)
{
    int i, d;
    // check TableIndex
    if (m_nvar == 0 || TabIndex < 1 || TabIndex > m_ntab) {
		return false;
    }

    // check number of dimensions
    if (nDim < 1 || nDim > MAXDIM) {
		return false;
    }

    // check BIRWeightVarIndex
    if (IsBIR) {
        if (BIRWeightVarIndex < 1 || BIRWeightVarIndex > m_nvar || !m_var[BIRWeightVarIndex - 1].IsWeight) {
            return false;
        }
    }

    // check variable indices, variable should be categorical
    for (i = 0; i < nDim; i++) {
        d = VarList[i]; // index of variable, 1 .. m_nvar is oke
        if (d < 1 || d > m_nvar) {
		return false;
        }
        if (!m_var[d - 1].IsCategorical) { // property set in SetVariable(...)
            return false;
        }
    }

    // make zero based
    i = TabIndex - 1;

    // insert table properties
    m_tab[i].Threshold = Threshold;

    //////////////////////
    //m_tab[i].nDim = nDim;
    //for (d = 0; d < nDim; d++) {
    //m_tab[i].Varnr[d] = VarList[d] - 1;
    // m_tab[i].SizeDim[d] = 0;  // yet unknown, after ExploreFile known
    //}
    ///////////////////////

    // check sequence vars, should be increasing with at least 1
    /////////////////////

    //////////////////////////////
    m_tab[i].BaseTable = true;
    if (IsBIR) {
        m_bHasBIR = true;
	m_lNumBIRs++;
	m_tab[i].IsBIR = true;
    }
    else {
        m_tab[i].IsBIR = false;
    }
    // m_tab[i].BIRWeightVar = BIRWeightVarIndex - 1;

    m_tab[i].SetVariables(nDim,VarList,BIRWeightVarIndex);
    if (!m_tab[i].CheckVarSequence() ) {
        return false;
    }
    m_tab[i].BIRThreshold = 0;  // of Threshold? AWTG 21-8-2001

    return true;
}

/**
 * Calculates the individual risk of records based on the frequency of key variable combination (fk)
 * and the sum of the weights of (Fk). Only important if BIR-table is specified (see SetTable).
 * If fk = 0, the result is 0. If fk = Fk = 1, the result is 1.
 * This function is called by MakeSafeFile. The reason for this function as an export function to include
 * is the fact that it is nice to have a complicated calculation process at your disposal.
 * See "Strategy for the Implementation of individual risk methodology to write-ARGUS: independent 
 * (and hierarchical) units": Maurizio Bianchi and Alessandra Capo Lucarelli, ISTAT, MPS / D, 
 * Via C. Bilbao 16, 00184, Roma, Italy, Deliverable No: D1-1.2 May 30, 2001 (third draft)
 * @param fk    0,1,2,... frequency of key variable combination
 * @param Fk    Total weights of the records with this key variable combination
 * @param risk  The risk of this combination, 0 <= risk <= 1
 * @return false if parameters are wrong (fk < 0, Fk < 0)
 */
bool CMuArgCtrl::BaseIndividualRisk(long fk, double Fk, double *risk)
{
    double p; long r; double q; double x1; double x2; long i;

    *risk = 0;
    if (fk == 0) goto ready; // no frequency, no risk

    // parameters correct?
    if (fk < 0)	{
	return false;
    }
    if (Fk <= 0) {
	return false;
    }

    if (RISKMODEL == 1)  //het oude risk model
    {
        // parameters are oke
        if (fk >= Fk) {
            if (fk == 1 && Fk == 1) {
                *risk = 1;
                goto ready;
            }
            p = 0.999;
        } 
        else {
            p = fk / Fk;
        }

        r = fk;

        assert(p > 0 && p < 1);

        switch(r) {
            case 1:
                *risk = (p / ( 1 - p) ) * log(1 / p);
                goto ready;
            case 2:
            {
                double h = p / (1 - p);
                *risk = h - h * h * log(1 / p);
                goto ready;
            }
            default:
                if (r > 40) {
                    *risk = (double) p / (double)(r - 1 + p);
                    goto ready;
                }
                // r = 3, 4, ... 40
                {
                    double c1 = 1, c2 = 1;
                    int j = 0;
                    do {
                        double c;
                        c = - (pow(r - j - 1, 2) / (j + 1) ) * ((pow(p, j - r + 2) - 1) / (pow(p, j - r + 1) - 1) ) / (r - 2 - j);
                        c2 *= c;
                        c1 += c2;
                        j++;
                    } while ( j <= r - 3 && fabs(c2) >= 1E-15);
                    double pqr = exp(r * (log(p) - log(1 - p) ) );
                    *risk = ( ( ( pow(1 / p, r - 1) - 1) / (r - 1) ) * c1 + (r & 1 ? -1 : 1) * log(p) ) * pqr;
                    goto ready;
                }
        }
    }

    if (RISKMODEL == 2) // Nieuw risk model van Silvia en Luisa
    {
        // parameters are oke; fk > 0
        if (Fk <= fk) { // < kan eigenlijk niet gewichten zouden dan < 1 moeten zijn
                        // als Fk < fk dan eigenlijk Fk = fk beschouwen
            *risk = 1.0 / fk;
            goto ready;
        }
        else { // normale geval f < F
            p = fk / Fk;
        }

        r = fk; q = 1 - p;

        assert(p > 0 && p < 1);
        switch (r) {
            case 1:
                *risk = - log (p) * ( p / q ) ;
                goto ready;
            case 2:
                *risk = ( p * log(p) + q) * p / (q * q);
                goto ready;
            case 3:
                *risk = p * ( q * ( 3 * q - 2) - 2 * p * p *  log(p) ) / ( 2 * q * q * q);
                goto ready;
            default:
                x1 = 1;
                x2 = 1;
                for ( i = 1 ; i <= 7 ; i++ ) {
                    x2 = x2 * i * q / (fk + i);
                    x1 = x1 + x2;
                }
                *risk = x1 *  p / fk;
                goto ready;
        }
    }

    ready:
    assert(*risk >= 0 && *risk <= 1);
    return true;
}

/**
 * Calculates all with SetTable specified tables from the data file and all subtables 
 * thereof, e.g., for table ABC also subtables AB, AC, BC, A, B and C.
 * This also applies to the tables with the BIR property.
 * For each (sub)table the number of table cells with value in [0, threshold] is calculated.
 * All tables are stored in memory.
 * @param ErrorCode     
 *                      NOVARIABLES no variables specified
 *                      NOTABLES no tables specified
 *                      NOTENOUGHMEMORY all tables together have too much memory, at the moment
 *                      the limit is 50MB. This is to prevent the computer from swapping which 
 *                      unfortunately has unpleasant effects
 *                      NOTABLEMEMORY for a single table there is not enough memory
 *                      NODATAFILE there is no file specified to examine
 *                      FILENOTFOUND file can not be opened
 * @param TableIndex    Index of table where error occurred, -1 = no error
 * @return false in case of error
 */
bool CMuArgCtrl::ComputeTables(long *ErrorCode, long *TableIndex)
{
    long MemSizeAll = 0, MemSizeTable;
    int i,j;
    FILE *fd;
    char str[MAXRECORDLENGTH];

    // initialize errorcodes
    *ErrorCode = -1; // na
    *TableIndex = -1;  // na

    // Not the right moment, first call SetNumberVar
    if (m_nvar == 0) {
	*ErrorCode = NOVARIABLES;
        return false;
    }

    // Not the right moment, first call SetNumberTab
    if (m_ntab == 0) {
	*ErrorCode = NOTABLES;
        return false;
    }

    // First do ExploreFile
    if (m_fname[0] == 0) {
	*ErrorCode = NODATAFILE;
        return false;
    }

    // add SizeDim to tab
    for (i = 0; i < m_ntab; i++) {
        for (int d = 0; d < m_tab[i].nDim; d++) {
            m_tab[i].SizeDim[d] = m_var[m_tab[i].Varnr[d]].nCode;
	}
    }

    // Table returns a memory size
    // compute memory size for each table
    for (i = 0; i < m_ntab; i++) {
        MemSizeTable = m_tab[i].GetMemSize();
	MemSizeAll += MemSizeTable * sizeof(long);
	if (m_tab[i].IsBIR) {
            MemSizeAll += MemSizeTable * sizeof(double);
	}
    }

    // check total memory to use
    if (MemSizeAll > MAXMEMORYUSE) {
	*ErrorCode = NOTENOUGHMEMORY;
        return false;
    }

    for (i = 0; i < m_ntab; i++) {
	if(!m_tab[i].PrepareTable())	{
            *ErrorCode = NOTABLEMEMORY;
            *TableIndex = i + 1;
            return false;
	}
    }
    // make space for households
    if ((m_lNumberOfHH != 0) && (m_bHasBIR)) {
        m_HH = new CHousehold [m_lNumberOfHH];
	if (m_HH == 0) {
            // Not enough memory
            return false;
	}
	for (i= 0; i<m_lNumberOfHH; i++) {
            if (!m_HH[i].PrepareHouseholdBHR(m_lNumBIRs)) {
		return false;
            }
	}
    }
    fd = fopen(m_fname, "r");
    if (fd == 0) {
	*ErrorCode = FILENOTFOUND;
        return false;
    }

    int recnr = 0;
    //hier gaat de SAS variant wel goed AHNL 30 maart 2005
    while (!feof(fd) ) {
        int res = ReadMicroRecord(fd, str);
	if (++recnr % FIREPROGRESS == 0) {
            FireUpdateProgress((int)(ftell(fd) * 100.0 / m_fSize));  // for progressbar in container
        }
        switch (res) {
            case INFILE_ERROR:
                goto error;
                break;
            case INFILE_EOF:
                goto oke;
                break;
            case INFILE_OKE:
		if ((!m_InFileIsFixedFormat)&&(m_IgnoreFirstLine)&&(recnr == 1)) {
                    continue;
		}
		else {
                    FillTables(str);
                    break;
		}
        }
    }

    oke:
    // Once more to fill Households
    if ((m_lNumberOfHH>0) && (m_bHasBIR)) {
	rewind(fd);
	int res;
	if ((!m_InFileIsFixedFormat)&&(m_IgnoreFirstLine)) {
            res = ReadMicroRecord(fd, str);
	}

	bool newhousehold = false;
	long numberofmem = 0;
	long HHnum = 0;
	while (!feof(fd) ) {
            res = ReadMicroRecord(fd, str);
            if (++recnr % FIREPROGRESS == 0) {
                FireUpdateProgress((int)(ftell(fd) * 100.0 / m_fSize));  // for progressbar in container
            }
            switch (res) {
                case INFILE_ERROR:
                    goto error;
                    break;
                case INFILE_EOF:
                    m_HH[m_lNumberOfHH - 1].m_lNumberofMembers = numberofmem;
                    goto oke1;
                    break;
		case INFILE_OKE:
//                  if ((!m_InFileIsFixedFormat)&&(m_IgnoreFirstLine)&&(recnr == 1)) {
//				continue;
//			}
//			else {
				//Here find the number of members in a household
                    newhousehold = IsNewHH(str);
                    if (newhousehold) {
                        m_HH[HHnum].m_lNumberofMembers= numberofmem;
			numberofmem = 1;
			HHnum++;
                    }
                    else {
                        numberofmem ++;
                    }
                    break;
//		}
            }
        }
	m_HH[HHnum].m_lNumberofMembers= numberofmem;

    }
    else {
	// should I close fd;
    }

    oke1:
    fclose(fd);

    LastHHName = ""; CurrentHHName = "";
    ComputeSubTableList();
    return true;

    error:
    fclose(fd);
    return false;
}

bool CMuArgCtrl::ComputeTableIndex(char *str, CVariable *var, long Index)
{ 
    char code[MAXCODEWIDTH];

    bool IsMissing;
    std::string tempcode;
    if (m_InFileIsFixedFormat) {
        strncpy(code, (char *) &str[var->bPos], var->nPos);
	code[var->nPos] = 0;
    }
    else {
	if (ReadVariableFreeFormat(str,Index,&(tempcode))) {
            strcpy(code,(const char*)tempcode.c_str());
            code[var->nPos] = 0;
	}
    }
    tempcode = code;
    if (!(var->SetTableIndex(code))) {
        return false;
    }

    return true;
}

void CMuArgCtrl::FillTables(char *str)
{ 
    int i, j;
    double Weight = 0;

    // set TableIndex and recode in VARIABLES as na
    for (i = 0; i < m_nvar; i++) {
        m_var[i].TableIndex = -1;
        m_var[i].Recode.DestCode = 0;
    }

    // compute for each involved variable the table index
    for (i = 0; i < m_ntab; i++) {
        CTable *tab = &(m_tab[i]);
        for (j = 0; j < tab->nDim; j++) {
            CVariable *var = &(m_var[tab->Varnr[j]]);
            if (var->TableIndex < 0) { // first time, so compute index
                ComputeTableIndex(str, var, tab->Varnr[j]);
            }
        }
    }

    // now tabulate all tables from list
    for (i = 0; i < m_ntab; i++) {
        if (m_tab[i].IsBIR) {
            int WeightVar = m_tab[i].BIRWeightVar;
            char code[MAXCODEWIDTH];
            std::string tempcode;

            assert(WeightVar >= 0 && WeightVar < m_nvar);
            // Here Change code
            if (m_InFileIsFixedFormat) {
                strncpy(code, (char *) (&str[m_var[WeightVar].bPos]), m_var[WeightVar].nPos);
		code[m_var[WeightVar].nPos] = 0;
            }
            else {
                if (ReadVariableFreeFormat(str,WeightVar,&(tempcode))) {
                    strcpy(code,(const char*)tempcode.c_str());
                    code[m_var[WeightVar].nPos] = 0;
		}
            }
            Weight = atof(code);
        } else {
            Weight = 0;
        }
        AddTableCell(m_tab[i], Weight);
    }
}

void CMuArgCtrl::AddTableCell(CTable& t, double Weight)
{
    int i, cellindex = 0;
    int SizeDim[MAXDIM];

    // compute size of all dims
    for (i = 0; i < t.nDim; i++) {
        SizeDim[i] = m_var[t.Varnr[i]].nCode;
    }

    // compute cellindex
    for (i = 0; i < t.nDim; i++) {
        cellindex *= SizeDim[i];
        cellindex += m_var[t.Varnr[i] ].TableIndex; // index in table of code for each variable
        // if (i < t.nDim - 1) cellindex *= SizeDim[i + 1];
    }

    assert(cellindex >= 0 && cellindex < t.nCell);
    t.Cell[cellindex]++;
    if (t.IsBIR) {
        t.BIRCell[cellindex] += Weight;
    }
}

int CMuArgCtrl::ComputeSubTableList()
{
    int i;
    int vars[MAXDIM];
    int dim, maxdim;

    // free results previous action
    // may be you don't need this becoz UCList is removed and thus tables
    for (i = 0; i < m_nUC; i++) {
        if (m_UCList[i].nDim != m_tab[m_UCList[i].TabNr].nDim) {  // no base table
            delete[] m_UCList[i].table.Cell;
            m_UCList[i].table.Cell = 0;
            m_UCList[i].table.nCell = 0;
            if (m_UCList[i].table.IsBIR) {
                if (m_UCList[i].table.BIRCell != 0) {
                    delete[] m_UCList[i].table.BIRCell;
                    m_UCList[i].table.BIRCell = 0;
		}
            }
	}
    }

    if (m_unsafe != 0) {
        delete [] m_unsafe;
	m_unsafe = 0;
    }

    m_unsafe = new int[1][MAXDIM + 1];
    if (m_unsafe == 0) {
        return -1;
    }
    memset(m_unsafe, 0, sizeof(int) * (MAXDIM + 1));
    
    m_cuc = m_nUC = 0;

    // first compute number of (sub)tables
    for (i = 0; i < m_ntab; i++) {
    // if (m_tab[i].IsBIR) continue; // no subtables for BIR tables, or???
        m_nUC += (int)(pow(2, m_tab[i].nDim) + .001) - 1;
    }

    // free previous action
    if (m_UCList != 0) { // not the first time
        delete [] m_UCList;
    }

    // allocate memory for it
    m_UCList = new CUCList[m_nUC];
    if (m_UCList == 0) {
        return -1;
    }

    for (i = 0; i < m_ntab; i++) {
    // if (m_tab[i].IsBIR) continue; // no subtables for BIR tables, or???
        DoSubTableList(i, 0, 0, vars, -1);
    }
    assert(m_cuc <= m_nUC);
    m_nUC = m_cuc;

    // compute max dim
    maxdim = 0;
    for (i = 0; i < m_nUC; i++) {
        if (m_UCList[i].nDim > maxdim) {
            maxdim = m_UCList[i].nDim;
	}
    }

    CTable t;
    int nTables = 0;
    // compute subtables
    for (dim = maxdim - 1; dim > 0; dim--) {
        for (i = 0; i < m_nUC; i++) {
            CUCList *u = &(m_UCList[i]);     // for easier and faster reference
            if (u->nDim != dim) continue;
            if (dim == m_tab[u->TabNr].nDim) continue;
            // fill subtable data
            t.nDim = dim;
            for (int j = 0; j < dim; j++) {
                t.Varnr[j] = u->Varnr[j];
                t.SizeDim[j] = m_var[t.Varnr[j]].GetnCodes(true);
            }
            t.Threshold = u->Threshold;
            t.BaseTable = false;
            // find a table with one dimension more that contains the same variables
            for (int k = 0; k < m_nUC; k++) {
                if (dim + 1 != m_UCList[k].nDim) continue;
                if (dim + 1 == m_tab[m_UCList[k].TabNr].nDim) {  // basis table is a permanent table
                    if (ComputeSubTable(m_tab[m_UCList[k].TabNr], t) > 0)  {
                        FireUpdateProgress( (int) (++nTables * 100.0 / m_nUC));
                        break;
                    }
                }
                else {
                    if (dim + 1 == m_UCList[k].nDim) {             // basis table is a subtable
                        if (ComputeSubTable(m_UCList[k].table, t) > 0)  {  // an earlier computed table
                            break;
                        }
                    }
                }
            }
            //assert(k < m_nUC); // table created
            assert(m_UCList[i].nDim > 0 &&  m_UCList[i].nDim <= MAXDIM);
            m_unsafe[0][m_UCList[i].nDim] = 0;
            m_UCList[i].table = t;  // save table in UCList
            m_UCList[i].nUC = m_unsafe[0][m_UCList[i].nDim]; // not for every code
        }
    }

    FireUpdateProgress(100);
    /// print tables out from here

    // compute unsafe cells in m_unsafe for every table
    for (i = 0; i < m_nUC; i++) {
        CUCList *u = &(m_UCList[i]);
        m_unsafe[0][u->nDim] = 0;
	if (u->nDim == m_tab[u->TabNr].nDim) { // base table
            ComputeUnsafeCells(m_tab[u->TabNr], -1);
	}
	else {
            ComputeUnsafeCells(u->table, -1);
	}
	u->nUC = m_unsafe[0][u->nDim]; // not for every code
    }
    // sort UCList on identical variable combinations,
    // equals have largest threshold first
    SortUCList(m_nUC, m_UCList);

    CUCList temp = m_UCList[0];
    m_UCList[0].biggestThreshold = true;
    for (i = 1; i < m_nUC; i++)
    {
        while (i < m_nUC && CompareUCList(temp, m_UCList[i]) == 0) {
            m_UCList[i].biggestThreshold = false;
            i++;
	}
	if (i <m_nUC)
	{
            temp = m_UCList[i];
            m_UCList[i].biggestThreshold = true;
	}
    }
    return maxdim;
}

void CMuArgCtrl::DoSubTableList(int iTab, int niv, int from, int *vars, int CVar)
{
    int i, j;

    for (i = from; i < m_tab[iTab].nDim; i++) {
        CUCList uc;
        //memset(&uc, 0, sizeof(UCLIST) );
        if (m_tab[iTab].HasRecode) {
            uc.TabNr = iTab + m_ntab; // take the recoded table!
        } else {
            uc.TabNr = iTab;  // take the permanent table
        }
        uc.nDim = niv + 1;
        uc.nUC = 0;  // will be filled later
        uc.Threshold = m_tab[iTab].Threshold;
        for (j = 0; j < niv; j++) {
            uc.Varnr[j] = vars[j];
        }
        uc.Varnr[niv] = m_tab[iTab].Varnr[i];
        assert(m_cuc >= 0 && m_cuc < m_nUC);

        // if CVar >= 0: CVar should be in uc.Varnr[]
        for (j = 0; j <= niv; j++) {
            if (CVar < 0) break;
            if (uc.Varnr[j] == CVar) break;
        }

	//----------------------

	//add memory for the
	//----------------------
        if (j <= niv ) {
            m_UCList[m_cuc++] = uc;  // add to list
        }

        // to next level
        vars[niv] = m_tab[iTab].Varnr[i];
        DoSubTableList(iTab, niv + 1, i + 1, vars, CVar);
    }

}

int CMuArgCtrl::ComputeSubTable(CTable &BaseTable, CTable &SubTable)
{
    int i, j;
    int IsSubVar[MAXDIM];  // for each base variable true or false for "in table"

    // is indeed a SUBtable?
    assert(SubTable.nDim > 0 && SubTable.nDim < BaseTable.nDim);
    if (SubTable.nDim >= BaseTable.nDim) { // that's no sub
        return -SUBTABLENOSUB;
    }
    // initialize all base table variables on false
    memset(IsSubVar, false, sizeof(IsSubVar) );

    // set subtable variables on true
    for (i = 0; i < SubTable.nDim; i++) {
        for (j = 0; j < BaseTable.nDim; j++) {
            if (BaseTable.Varnr[j] == SubTable.Varnr[i]) {
                IsSubVar[j] = true;
		break;
            }
	}
	if (j == BaseTable.nDim) {  // varnr not in table, error
            return -SUBTABLEWRONGVAR;
	}
    }

    SubTable.nCell = 1;
    for (i = 0; i < SubTable.nDim; i++) {
        SubTable.nCell *= SubTable.SizeDim[i];
    }
    if (SubTable.IsBIR = BaseTable.IsBIR) {
        SubTable.BIRThreshold = BaseTable.BIRThreshold;
	SubTable.BIRWeightVar = BaseTable.BIRWeightVar;
    }

    if (!SubTable.PrepareTable()) {
        return -NOTENOUGHMEMORY;
    }

    MakeSubTable(BaseTable, SubTable, 0, 0, 0, IsSubVar);

    return true;
}

void CMuArgCtrl::MakeSubTable(CTable& BaseTab, CTable& SubTab, int niv, int iParentCell, int iSubCell, int *tabvars)
{
    int i, n, v;

    if (niv == BaseTab.nDim) {  // basic level, so count
        assert(iSubCell >= 0 && iSubCell < SubTab.nCell);
	assert(iParentCell >= 0 && iParentCell < BaseTab.nCell);
	SubTab.Cell[iSubCell] += BaseTab.Cell[iParentCell];

	// also for BIR
	if (BaseTab.IsBIR) {
            SubTab.BIRCell[iSubCell] += BaseTab.BIRCell[iParentCell];
	}

	return;
    }

    v = BaseTab.Varnr[niv];     // index of variable niv
    n = BaseTab.SizeDim[niv];   // number of codes
    
    for (i = 0; i < n; i++) {
        int iSub, iParent;
	// compute iSub: index in subtable
	iSub = iSubCell;
	if (tabvars[niv]) {  // variable is in subtable
            iSub *= BaseTab.SizeDim[niv];
            iSub += i;
	}
	// compute iParent: index in parent table
	iParent = iParentCell;
	iParent *= BaseTab.SizeDim[niv];
	iParent += i;

	MakeSubTable(BaseTab, SubTab, niv + 1, iParent, iSub, tabvars);
    } // forloop with i = index code of a variable
}

bool CMuArgCtrl::ComputeUnsafeCells(CTable & t, int CVar)
{
    ComputeNumberUnsafeCells(t, 0, 0, false, CVar);
    return true;
}

void CMuArgCtrl::ComputeNumberUnsafeCells(CTable & t, int niv, int cindex, bool IsMissingCode, int CVar, int code)
{
    assert(CVar < m_nvar);
    //assert( (CVar < 0 && code < 0) || (CVar >= 0 && code >= -1 && code < m_var[CVar].nCode) );

    if (niv == t.nDim) {
        assert(cindex >= 0 && cindex < t.nCell);
	assert(CVar < 0 || code >= 0);
	if (t.Cell[cindex] <= t.Threshold && t.Cell[cindex] != 0 && !IsMissingCode) {
            if (CVar < 0) {
                m_unsafe[0][niv]++;
            }
            else {
                m_unsafe[code][niv]++;
            }
	}
	if (CVar >= 0 && t.nDim == 1) {  // save freq
            m_unsafe[code][0] = t.Cell[cindex];
	}
	return;
    }

    int i, c;
    int v = t.Varnr[niv];
    int n, NMis;
    bool Mis;
    CVariable *var;
    var = &(m_var[t.Varnr[niv]]);
    
    // compute number of codes of variable v, can be recoded!
    //n = GetnCodes(v, true);
    n = var->GetnCodes(true);
//	if (CVar < 0) {  // don't count missings for left pane
// even kortgesloten Anco 21/1/2005
    if (m_var[v].HasRecode) {
//			n -= m_var[v].Recode.nMissing;
        NMis = n - m_var[v].Recode.nMissing;
    }
    else {
//			n -= m_var[v].nMissing;
        NMis = n - m_var[v].nMissing;
    }
//	}

    for (i = 0; i < n; i++) {
        c = cindex;
	c *= t.SizeDim[niv];
	c += i;
	if (v == CVar) code = i;
        Mis = (i >= NMis)  || IsMissingCode;
        ComputeNumberUnsafeCells(t, niv + 1, c, Mis, CVar, code);
    }
}

void CMuArgCtrl::SortUCList(int n, CUCList *uc)
{
    if (n > 1) {
        QuickSortUCList(uc, 0, n - 1);
    }
}

void CMuArgCtrl::QuickSortUCList(CUCList *s, int first, int last)
{
    int i, j;
    CUCList mid, temp;

    assert(first >= 0 && last >= first);

    do {
        i = first;
        j = last;
        mid = s[(i + j) / 2];
        do {
            while (CompareUCListThres(s[i], mid) < 0) i++;
            while (CompareUCListThres(s[j], mid) > 0) j--;
            if (i < j) {
                temp = s[i];
                s[i] = s[j];
                s[j] = temp;
            } 
            else {
                if (i == j) {
                    i++;
                    j--;
                }
                break;
            }
        } while (++i <= --j);

        if (j - first < last - i) {
            if (j > first) {
                QuickSortUCList(s, first, j);
            }
            first = i;
        }
        else {
            if (i < last) {
                QuickSortUCList(s, i, last);
            }
            last = j;
        }
    } while (first < last);
}

int CMuArgCtrl::CompareUCListThres(CUCList& a, CUCList& b)
{
    int v;

    v = memcmp(a.Varnr, b.Varnr, sizeof(a.Varnr) );
    if (v != 0) return v;
    //return (a.Threshold < b.Threshold);
    return b.Threshold - a.Threshold;
}

int CMuArgCtrl::CompareUCList(CUCList& a, CUCList& b)
{
    return memcmp(a.Varnr, b.Varnr, sizeof(a.Varnr) );
}

/**
 * Calculates the maximum number nUC of the permanent (sub)tables
 * @return long Maximum number nUC of the permanent (sub)tables
 */
long CMuArgCtrl::GetMaxnUC()
{
    int i, max = 0;

    // too early?
    if (m_nvar == 0 || m_ntab == 0 || m_fname[0] == 0) {
	return -1;
    }

    for (i = 0; i < m_nUC; i++) {
        if (m_UCList[i].nUC > max) {
            max = m_UCList[i].nUC;
	}
    }
    return max;
}

/**
 * Calculates for each relevant dimension (1,2,...) the number of unsafe combinations (UC) for a variable
 * @param VarIndex  Index of the variable
 * @param Count     Number of elements in UCArray
 * @param UCArray   Array of UCs of dimensions 1, 2, ..., Count
 * @return false if one or more parameters is wrong
 */
bool CMuArgCtrl::UnsafeVariable(long VarIndex, long *Count, long *UCArray)
{
    int ndim, i, j, nUnsafe, v = VarIndex - 1, counter;
    bool tabsfound, varfound = false;

    // Not the right moment, first call SetNumberVar
    if (m_nvar == 0)  {
	return false;
    }
    if (v < 0 || v >= m_nvar || !m_var[v].IsCategorical) {
    	return false;
    }
    // There are tables generated?
    if (m_ntab == 0) {
	return false;
    }

    counter = 0;
    for (ndim = 1; ndim < MAXDIM; ndim++) {
        nUnsafe = 0;
	tabsfound = false;
	for (i = 0; i < m_nUC; i++) {
            if (!m_UCList[i].biggestThreshold) continue;  // = same variable combination with smaller threshold
            int nTabDim = m_UCList[i].nDim;
            if (nTabDim == ndim) {
                for (j = 0; j < ndim; j++) {
                    if (m_UCList[i].Varnr[j] == v) break;  // Variable in list?
		}
		if (j < ndim) {  // yes
                    varfound = true;
                    tabsfound = true;
                    nUnsafe += m_UCList[i].nUC;
		}
            }
	}
	if (tabsfound) {
            UCArray[counter++] = nUnsafe;
	}
	else {
            break;
	}
    }

    if (!varfound) {
	return false;  // Variable not present in any table
    }

    *Count = counter;

    return true;
}

/**
 * Prepares for a variable the information by code
 * In m_unsafe[nCode][MAXDIM+1] is stored for each code:
 *      frequency (at index 0)
 *      for each relevant dimension the number of UCs (at index 1, 2, ...)
 * Any action not completed any other variable is reversed so UnsafeVariableClose is
 * not necessarily required
 * @param VarIndex  Index of variable
 * @param nCode     Number of codes of the variable, including Missing
 * @return false if one or more parameters is wrong
 */
bool CMuArgCtrl::UnsafeVariablePrepare(long VarIndex, long *nCode)
{
    int v = VarIndex - 1;
    int i, j, nCodes;
    bool varfound;

    // Not the right moment, first call SetNumberVar
    if (m_nvar == 0)	{
	return false;
    }
    // VarIndex correct?
    if (v < 0 || v >= m_nvar || !m_var[v].IsCategorical) {
	return false;
    }
    // There are tables generated?
    if (m_ntab == 0)	{
        return false;
    }

    // Variable in any table?
    varfound = false;
    for (i = 0; i < m_ntab && !varfound; i++) {
	//if (m_tab[i].IsBIR) continue; // no subtables for BIR tables, or???
	for (j = 0; j < m_tab[i].nDim && !varfound; j++) {
            if (m_tab[i].Varnr[j] == v) {
                varfound = true;
            }
	}
    }

    if (!varfound) {
	return false;
    }

    // Now try to find for variable 'v'
    // for all dims and all codes of 'v' the unsafe combinations
    // save results in m_unsafe[nCode][MAXDIM + 1]
    CVariable *var;
    var = &(m_var[v]);
    nCodes = var->GetnCodes(true);

    if (m_unsafe != 0) {
        delete [] m_unsafe;
	m_unsafe = 0;
    }

    m_unsafe = new int[nCodes][MAXDIM + 1];
    if (m_unsafe == 0) {
	// Not too sure about this
	return false;
    }
    memset(m_unsafe, 0, sizeof(int) * (MAXDIM  + 1) * nCodes);

    m_maxdim = 0;
    for (i = 0; i < m_nUC; i++) {
        CUCList *ul = &(m_UCList[i]);
	if (!ul->biggestThreshold) continue;
	int n = ul->nDim;
	for (j = 0; j < n; j++) {
            if (ul->Varnr[j] == v) {
                if (n == m_tab[ul->TabNr].nDim) { // basetable
                    ComputeUnsafeCells(m_tab[ul->TabNr], v);
		}
                else {
                    ComputeUnsafeCells(ul->table, v);
		}
		if (n > m_maxdim) {
                    m_maxdim = n;
		}
		break;
            }
	}
    }
    var = &(m_var[v]);
    *nCode = var->GetnCodes(true);
    return true;
}

/**
 * Indicates a variable frequency code index, code and number of UCs per dimension
 * UnsafeVariablePrepare should be invoked first with the same index variable
 * @param VarIndex  Index of variable
 * @param CodeIndex Index of code
 * @param IsMissing 1 if Missing code, 0 otherwise
 * @param Freq      frequency of the code
 * @param Code      Alphanumeric code
 * @param Count     Number of elements in UCArray
 * @param UCArray   Array of UCs of dimensions 1, 2, ..., Count
 * @return false if one or more parameters is wrong
 */
bool CMuArgCtrl::UnsafeVariableCodes(long VarIndex, long CodeIndex, long *IsMissing, long *Freq, const char **Code, long *Count, long *UCArray)
{
    int v = VarIndex - 1;
    int c = CodeIndex - 1;
    int i, n;

    // Not the right moment, first call SetNumberVar
    if (m_nvar == 0) {
	return false;
    }

    // VarIndex correct?
    if (v < 0 || v >= m_nvar || !m_var[v].IsCategorical) {
	return false;
    }

    // CodeIndex correct? Missing is also a valid code? Yes (AHNL 19-3-2001)
    n = m_var[v].GetnCodes(true);
    if (c < 0 || c >= n) {
	return false;
    }

    n = m_var[v].GetnCodes(false);
    //*IsMissing = (c >= n);
    if (c>= n) {
        *IsMissing = 1;
    }
    else {
        *IsMissing = 0;
    }

    *Freq = m_unsafe[c][0]; // 'zero' dimension = freq

    if (m_var[v].HasRecode) {
        *Code = m_var[v].Recode.sCode[c].c_str();
    }
    else {
        *Code = m_var[v].sCode[c].c_str();
    }

    *Count = m_maxdim;
    
    for (i = 1; i <= m_maxdim; i++) {
        UCArray[i - 1] = m_unsafe[c][i];
    }

    return true;
}

/**
 * Frees memory reserved in UnsafeVariablePrepare
 * @param VarIndex  Index of variable
 * @return false if parameter is wrong
 */
bool CMuArgCtrl::UnsafeVariableClose(long VarIndex)
{
    int v = VarIndex - 1;

    // VarIndex correct?
    if (v < 0 || v >= m_nvar || !m_var[v].IsCategorical) {
	return false;
    }

    if (m_unsafe != 0) {
        delete [] m_unsafe;
	m_unsafe = 0;
    }

    return true;
}

/**
 * Reduces the number of codes of a variable by grouping several codes together. 
 * The RecodeString are separated by newline characters(\r \n).
 * Lines are of the form "object codes : source codes"
 * Source codes that are too short are expanded through spaces in front of it.
 * Codes may be surrounded by quotation marks ("3 - "), it may be necessary if
 * the end code ends with a space or a comma as a code or contains a dash. 
 * Examples:
 * 0 - 90                   All codes between 0 and 90 (inclusive)
 * 1: 91-500                New code 1 is old codes 91 to 500 (inclusive)
 * 2: 501 -                 New code 2 is old codes >= 501
 * 3: 11, "3", 512, 530-570 New code 3 is combination of multiple series, separated by ','
 * 
 * A code is always treated alphanumerical. Therefore, the following comparisons hold:
 * "1a" > "19"
 * "1 " < "11"
 * 
 * @param VarIndex      Index of variable
 * @param RecodeString  Specification of string of the recoding
 * @param eMissing1     Value of Missing1
 * @param eMissing2     Value of Missing2
 * @param ErrorType     Wrong type
 * @param ErrorLine     Line where error occurred
 * @param ErrorPos      Position where error occurred in line
 * @param WarningString Warning of overlapping sources, non-mentioned codes, etc.
 * @return false if one or more parameters is wrong
 */
bool CMuArgCtrl::DoRecode(long VarIndex, const char *RecodeString, const char *eMissing1, const char *eMissing2, long *ErrorType, long *ErrorLine, long *ErrorPos, const char **WarningString)
{
    int i, v = VarIndex - 1, oke, maxwidth = 0;
    *ErrorType = *ErrorLine = *ErrorPos = -1;
    
    //std::string temp, Missing1, Missing2, sRecodeString;
    std::string Missing1, Missing2;

    Missing1 = eMissing1;
    Missing2 = eMissing2;
    
    // for warnings
    m_nOverlap = 0;
    m_nUntouched = 0;
    m_nNoSense = 0;

    m_WarningRecode.resize(0);

    // too early?
    if (m_nvar == 0 || m_ntab == 0 || m_fname[0] == 0) {
    	*ErrorType =  E_NOVARTABDATA;
	return false;
    }

    // wrong VarIndex
    if (v < 0 || v >= m_nvar || !m_var[v].IsCategorical) {
	*ErrorType =  E_VARINDEXWRONG;
	return false;
    }

    if (m_var[v].HasRecode) {
    	m_var[v].UndoRecode();
    }

    // only check syntax, phase = CHECK
    oke = ParseRecodeString(v, RecodeString, ErrorType, ErrorLine, ErrorPos, CHECK);
    if (!oke) {
	return false;
    }

    if (!m_var[v].PrepareRecode()) {
	*ErrorType = NOTENOUGHMEMORY;
	return false;
    }
    if (Missing1.empty() && !Missing2.empty()) {
	Missing1 = Missing2;
    }

    // no missings for recode specified? Take the missing(s) of the source
    if (Missing1.empty() && Missing2.empty()) {
	Missing1 = m_var[v].Missing1;
	if (m_var[v].nMissing == 2) {
            Missing2 = m_var[v].Missing2;
	}
	else {
            Missing2 = Missing1;
	}
    }
    ParseRecodeString(v, RecodeString, ErrorType, ErrorLine, ErrorPos, DESTCODE);
    if (m_var[v].Recode.sCode.size() < 1) {
	*ErrorType = E_EMPTYSPEC;
	*ErrorLine = 1;
	*ErrorPos = 1;
	return false;
    }

    // sort list of dest codes, still without missing values (coming soon)
    m_var[v].SortRecodedCode( 0, m_var[v].Recode.sCode.size() - 1);

    // now the number of codes is known, but not the not mentioned ones
    // m_var[v].Recode.nCode = m_var[v].Recode.sCode.GetSize();
    // again, now compute dest codes and link between dest and src
    oke = ParseRecodeString(v, RecodeString, ErrorType, ErrorLine, ErrorPos, SRCCODE);
    if (!oke) {
	return false;  // missing to valid codes, a terrible shame
    }

    // compute untouched codes, add them to the recode codelist
    m_nUntouched = 0;
    for (i = 0; i < m_var[v].nCode - m_var[v].nMissing; i++) {
	if (m_var[v].Recode.DestCode[i] == -1) { // not touched
            m_nUntouched++;
            m_var[v].AddRecode(m_var[v].sCode[i].c_str());
	}
    }
    // make all recode codes same width
    maxwidth = m_var[v].MakeRecodelistEqualWidth(Missing1, Missing2);

    // remove the missing codes, if present
    {
	int n;
	bool IsMissing;
	std::string mis = Missing1;
	AddSpacesBefore(mis, maxwidth);
	if (n = m_var[v].FindRecodedCode(mis, 0, IsMissing), n >= 0) {
            //m_var[v].Recode.sCode.RemoveAt(n);
            m_var[v].Recode.sCode.erase(m_var[v].Recode.sCode.begin() + n);
	}
	mis = Missing2;
	AddSpacesBefore(mis, maxwidth);
	if (n = m_var[v].FindRecodedCode(mis, 0, IsMissing), n >= 0) {
            //m_var[v].Recode.sCode.RemoveAt(n);
            m_var[v].Recode.sCode.erase(m_var[v].Recode.sCode.begin() + n);
	}
    }
    // sort list of dest codes, still without missing values (coming soon)
    m_var[v].SortRecodedCode( 0, m_var[v].Recode.sCode.size() - 1);

    // ADD MISSING1 AND -2
    // both empty impossible, see start of function
    // swap missings if missing1 empty

    if (Missing1.empty() && Missing2.empty()) {             // no missing specified?
	m_var[v].Recode.Missing1 = m_var[v].Missing1;  // take the missing of source variable
	m_var[v].Recode.Missing2 = m_var[v].Missing2;
    }
    else {
        if (Missing1.empty()) {  // at least one missing specified
            m_var[v].Recode.Missing1 = Missing2;
            m_var[v].Recode.Missing2 = Missing1;
	}
	else {
            m_var[v].Recode.Missing1 = Missing1;
            m_var[v].Recode.Missing2 = Missing2;
	}
    }

    // second empty?
    if (m_var[v].Recode.Missing2.empty()) {
        m_var[v].Recode.Missing2 = m_var[v].Recode.Missing1;
	m_var[v].Recode.nMissing = 1;
    }
    else {
	// equal?
        if (m_var[v].Recode.Missing1 == m_var[v].Recode.Missing2) {
            m_var[v].Recode.nMissing = 1;
	}
	else {
            m_var[v].Recode.nMissing = 2;
	}
    }

    // put in list, last one or two
    AddSpacesBefore(m_var[v].Recode.Missing1, maxwidth);
    m_var[v].AddRecode(m_var[v].Recode.Missing1.c_str());
    if (m_var[v].Recode.nMissing == 2) {
        AddSpacesBefore(m_var[v].Recode.Missing2, maxwidth);
	m_var[v].AddRecode(m_var[v].Recode.Missing2.c_str());
    }

    // last time, now compute dest codes and link between dest and src
    // for warnings
    m_nOverlap = 0;
    m_nNoSense = 0;
    for (i = 0; i < m_var[v].nCode; i++) {
        m_var[v].Recode.DestCode[i] = -1;
    }
    oke = ParseRecodeString(v, RecodeString, ErrorType, ErrorLine, ErrorPos, SRCCODE);
    if (!oke) {
    	return false;  // missing to valid codes, a terrible shame
    }

    // yep, the number of codes is known and the codes are sorted (except one or two MISSINGs at the end of te list)
    m_var[v].Recode.nCode = m_var[v].Recode.sCode.size();

    // do the same for MISSINGS, more complicated
    { //RECODE *c = &(m_var[v].Recode); // for easier reference
        i = m_var[v].nCode - m_var[v].nMissing;     // first missing
	if (m_var[v].Recode.DestCode[i] == -1) {                 // missing 1 not specified
            m_var[v].Recode.DestCode[i] = m_var[v].Recode.nCode - m_var[v].Recode.nMissing;  // make missing1 equal to missing1 source
	}
	if (m_var[v].nMissing == 2) {               // two missings in source?
            i++;
            if (m_var[v].Recode.nMissing == 2) {                   // two missings in dest?
                if (m_var[v].Recode.DestCode[i] == -1) {                     // missing 2 not specified
                    m_var[v].Recode.DestCode[i] = m_var[v].Recode.nCode - m_var[v].Recode.nMissing + 1;  // make missing2 equal to missing2 source
		}
            }
            else {
                if (m_var[v].Recode.DestCode[i] == -1) {                 // missing 2 not specified
                    m_var[v].Recode.DestCode[i] = m_var[v].Recode.nCode - m_var[v].Recode.nMissing;  // make missing2 equal to missing1 source
		}
            }
	}
    }

    // set untouched codes on right index
    { //RECODE *c = &(m_var[v].Recode); // for easier reference
        bool IsMissing;
	int index, ncode = m_var[v].nCode - m_var[v].nMissing;
	for (i = 0; i < ncode; i++) {
            if (m_var[v].Recode.DestCode[i] == -1) {
                std::string str = m_var[v].sCode[i];
		AddSpacesBefore(str, m_var[v].Recode.CodeWidth);
		//index = BinSearchStringArray(c->sCode, str, c->nMissing, IsMissing);
		index = m_var[v].FindRecodedCode(str, m_var[v].Recode.nMissing, IsMissing);
		assert(index >= 0 && index < m_var[v].Recode.nCode);
		m_var[v].Recode.DestCode[i] = index;
            }
	}
    }

    // WARNINGS in recode:

    // show untouched codes:
    std::ostringstream ss;
    if (m_nUntouched > 0) {
        ss << "Number of untouched codes: " << m_nUntouched << "\r\n";
    }

    // show warnings
    if (m_nOverlap > 0) {
        ss << "Number of overlapping codes: " << m_nOverlap << "\r\n";
    }

    if (m_nNoSense > 0) {
        ss << "Number of \"no sense\" codes: " << m_nNoSense << "\r\n";
    }
    m_WarningRecode = ss.str();
    
    if (m_WarningRecode.empty() ) {
        m_WarningRecode = "Recode OK";
    }

    *WarningString = m_WarningRecode.c_str();
    //*WarningString = m_WarningRecode.AllocSysString();

    m_var[v].HasRecode = true;

    return true;
}

////////////////////////////////////////////////////////////////////////////
/// PARSE RECODE LIST
///  0 : - 90
///  1 : 90 - 500
///  2 : 500 -
///  3 : 11,13, 512, 530-570, 930-970
///  phase: CHECK       check only syntax
///         DESTCODE    compute and sort dest code
///         SRCCODE     compute link between src and dest code

bool CMuArgCtrl::ParseRecodeString(long VarIndex, const char *RecodeString, long *ErrorType, long *ErrorLine, long *ErrorPos, int Phase)
{ 
    const char *p;
    int PosInString = 0, LineNumber = 1;
    int oke;

    // first detect lines in RecodeString
    while (1) {
        oke = ParseRecodeStringLine(VarIndex, &RecodeString[PosInString], ErrorType,  ErrorPos, Phase);
        if (!oke) {
            *ErrorLine = LineNumber;
            return false;
        }
        LineNumber++;
        p = strstr(&RecodeString[PosInString], SEPARATOR);
        if (p == 0) break;
        PosInString = p - RecodeString + strlen(SEPARATOR);
    }

    return true;
}

// Parse a line of a recode string (until str[i] == 0 || str[i] == '\n' || str[i] == '\r')
bool CMuArgCtrl::ParseRecodeStringLine(long VarIndex, const char *str, long *ErrorType, long *ErrorPos, int Phase)
{ 
    int i = 0, len = strlen(str), res, fromto, position;
    int nPos = m_var[VarIndex].nPos;

    // the three codes
    char DestCode[MAXCODEWIDTH + 1];
    char SrcCode1[MAXCODEWIDTH + 1];
    char SrcCode2[MAXCODEWIDTH + 1];
    char Dummy[MAXCODEWIDTH+1];
    
    for (i = 0; i < len; i++) {
        if (str[i] > 32 || str[i] < 0) break;
        if (str[i] < 32) break;
    }

    if (i == len || str[i] < 32) {  // all characters useless, oke
        return true;
    }

    // now at least one character > 32
    // parse something like: 1 : 1 - 32 , 35 - 78, 99

    // first the destination code
    res = ReadWord(str, DestCode, Dummy, ':', fromto, position);

    if (res < 0 || str[res] != ':') {
        *ErrorType = E_HARD;
        *ErrorPos = position + i + 1;
        return false;
    }
    i += res;

    //if (strlen(DestCode) > (UINT) nPos) {
    //  *ErrorType = E_LENGTHWRONG;
    //  *ErrorPos = position + i + 1;
    //  return false;
    //}


    // now the source code(s)
    do {
        i++;
        res = ReadWord(&str[i], SrcCode1, SrcCode2, ',', fromto, position);
        if (res < 0) {
            *ErrorType = E_HARD;
            *ErrorPos = position + i + 1;
            return false;
        }

        // length oke?
        if (strlen(SrcCode1) > (unsigned int) nPos || strlen(SrcCode2) > (unsigned int) nPos) {
            *ErrorType = E_LENGTHWRONG;
            *ErrorPos = position + i + 1;
            return false;
        }

        AddSpacesBefore(SrcCode1, nPos);
        AddSpacesBefore(SrcCode2, nPos);

        // ScrCode2 smaller then SrcCode1?
        if (fromto == FROMTO_RANGE) {
            if (strcmp(SrcCode1, SrcCode2) > 0) {
                *ErrorType = E_RANGEWRONG;
                *ErrorPos = position + i + 1;
                return false;
            }
        }

        switch (Phase) {
            case DESTCODE:
                m_var[VarIndex].AddRecode(DestCode);
                break;
            case SRCCODE:
                {
                    int ErrCode = SetCode2Recode(VarIndex, DestCode, SrcCode1, SrcCode2, fromto);
                    if (ErrCode == R_MISSING2VALID) {   // a mortal sin
                        *ErrorType = ErrCode;
                        *ErrorPos = position + i + 1;
                        return false;
                    }
                }
                break;
        }

        i += res;

    } while (strchr("\n\r", str[i]) == 0 && str[i] != 0);

    return true;
}


int CMuArgCtrl::SetCode2Recode(int VarIndex, char *DestCode, char *SrcCode1, char *SrcCode2, int fromto)
{
    CVariable *v = &(m_var[VarIndex]);
    int c, c1 = 0, c2 = 0;
    int n_codes = v->sCode.size() - v->nMissing;
    int DestIndex, Exact;
    bool DestMissing, Src1Missing, Src2Missing;
    // compute index SrcCode1
    Exact = true;
    //if (c1 = BinSearchStringArray(v->sCode, SrcCode1, v->nMissing, Src1Missing), c1 < 0) { // not found?
    c1 = v->FindCode(SrcCode1, v->nMissing, Src1Missing);
    if (c1 <0) {
	Exact = false;
	if (v->sCode[n_codes - 1] < SrcCode1) {  // bigger then last code
            c1 = n_codes;
	}
	else {
            c1 = 0;
            while (m_var[VarIndex].sCode[c1] < SrcCode1) c1++;
	}
    }

    Src2Missing = false;
    switch (fromto) {
	case FROMTO_TO:
            c2 = c1;
            if (!Exact) c2--;
            c1 = 0;
            break;
	case FROMTO_SOLO:
            if (!Exact) {
		m_nNoSense++;
                return R_CODENOTINLIST;
            }
            c2 = c1;
            break;
	case FROMTO_FROM:
            c2 = n_codes - 1;
            break;
	case FROMTO_RANGE:
            if (fromto == FROMTO_RANGE) {   // compute index SrcCode2
                Exact = true;
		if (c2 = v->FindCode(SrcCode2, v->nMissing, Src2Missing), c2 < 0) { // not found?
                    Exact = false;
                    if (v->sCode[n_codes - 1] < SrcCode2) {  // bigger then last code, oke
                        c2 = n_codes;
                    }
                    else {
                        c2 = 0;
                        while (m_var[VarIndex].sCode[c2] < SrcCode2) c2++;
                    }
		}
		if (!Exact) c2--;
            }
            break;
	default:
            assert(1 == 2);
            return PROGRAMERROR;
    }

    if (c2 < c1) {
        m_nNoSense++;
	return R_NOSENSE;
    }
    // c1 and c2 now are correct, I assume
    assert(c1 >= 0 && c1 < v->sCode.size() && c2 >= c1 && c2 < v->sCode.size());

    // search destcode in list
    assert(v->nMissing > 0 && v->nMissing < 3);

    AddSpacesBefore(DestCode, v->Recode.CodeWidth);
    DestIndex = v->FindRecodedCode(DestCode, v->Recode.nMissing, DestMissing);

    // never make from a missing value a valid value!
    if (!DestMissing && (Src1Missing || Src2Missing) ) {
        return R_MISSING2VALID;
    }

    assert(DestIndex >= 0 && DestIndex < v->Recode.sCode.size() );
    if (DestIndex < 0 || DestIndex >= v->Recode.sCode.size() ) {
        return PROGRAMERROR;
    }

    // put dest code
    for (c = c1; c <= c2; c++) {
        int d = v->Recode.DestCode[c];
	if (d != -1) {
            m_nOverlap++;
	}
	v->Recode.DestCode[c] = DestIndex;
    }

    return 0;
}

int CMuArgCtrl::ReadWord(const char *str, char* CodeFrom, char *CodeTo, char EndCode, int& fromto, int& pos)
{ 
    int i = 0, j, pass = 0;
    char *p;

    fromto = FROMTO_SOLO;
    pos = 0;
    CodeFrom[0] = 0;
    if (CodeTo) {
        CodeTo[0] = 0;
    }

    do {
        switch (++pass) {
            case 1:
                p = CodeFrom;
                break;
            case 2:
                if (fromto == FROMTO_SOLO) {  // in the case: 1 2, one should use a hyphen between the codes
                    pos = i;
                    return -1;
                }
                p = CodeTo;
                break;
            default:
                pos = i;
                return -1;
        }

        j = 0;
        while (str[i] == ' ') i++;  // skip spaces before word or hyphen

        // a hyphen before?
        if (str[i] == '-') {
            if (fromto == FROMTO_SOLO) {  // first hyphen?
                switch (pass) {
                    case 1:
                        fromto = FROMTO_TO;
                        break;
                    default:
                        pos = i;
                        return -1;
                }
            } else {
                pos = i;
                return -1;  // more than one hyphen
            }
            i++;
            while (str[i] == ' ') i++;  // skip spaces between - and word
        }

        if (fromto == FROMTO_FROM && pass == 2) {
            fromto = FROMTO_RANGE;
        }

        // do the code
        if (str[i] == '"') {  // first character double quote?
            i++;
            while (str[i] != '\0' && strchr("\n\r\"", str[i]) == 0) {
                if (j < MAXCODEWIDTH) {
                    p[j++] = str[i++];
                } else {
                    pos = i;
                    return -1;
                }
            }
            if (str[i] != '"') { // end double quote not found
                pos = i;
                return -1;
            }
            i++;
            // not a starting double quote
        } else {
            while (str[i] != '\0' && str[i] != EndCode && strchr(" -\n\r", str[i])== 0) {
                if (j < MAXCODEWIDTH) {
                    p[j++] = str[i++];
                } else {
                    pos = i;
                    return -1;
                }
            }
        }
        p[j] = 0;
        if (j == 0) { // empty
            pos = i;
            return -1;
        }

        while (str[i] == ' ') i++; // skip spaces after word

        // an ending hyphen?
        if (str[i] == '-') {
            if (fromto == FROMTO_SOLO) {
                switch (pass) {
                    case 1:
                        fromto = FROMTO_FROM;
                        break;
                    default:
                        pos = i;
                        return -1;
                }
                i++;
            } else {
                pos = i;
                return -1;  // more than one -
            }
        }

        while (str[i] == ' ') i++; // skip spaces after word

    } while (str[i] != EndCode && str[i] != '\0' && str[i] != '\n' && str[i] != '\r');

    return i; // oke, return current position
}

/**
 * Undoes a recoding. There cannot be a re-encoding applied on a recoded variable.
 * A recoding is always applied on the basic code list of a variable. 
 * A new recoding can be done without calling this function first.
 * The results can be retrieved using
 * UnsafeVariable
 * UnsafeVariablePrepare
 * UnsafeVariableCodes
 * UnsafeVriableClose
 * @param VarIndex  Index of the variable
 * @return false if parameter in wrong
 */
bool CMuArgCtrl::UndoRecode(long VarIndex)
{   
    int v = VarIndex - 1;

    if (m_nvar == 0 || m_ntab == 0 || m_fname[0] == 0) {
        return false;
    }

    // wrong VarIndex
    if (v < 0 || v >= m_nvar || !m_var[v].IsCategorical) {
        return false;
    }

    // set variable without recode, only change the flag
    m_var[v].HasRecode = false;

    // recomputes for all tables the flag HasRecode
    SetTableHasRecode();

    return true;
}

void CMuArgCtrl::SetTableHasRecode()
{
    int t, v;

    for (t = 0; t < m_ntab; t++) {
        for (v = 0; v < m_tab[t].nDim; v++) {
            if (m_var[m_tab[t].Varnr[v]].HasRecode) {
                break;
            }
	}
	m_tab[t].HasRecode = (v != m_tab[t].nDim);
    }
}

/**
 * Reduces the number of codes of a variable by cutting off one or more positions at the right
 * @param VarIndex  Index of variable
 * @param nPos      Number of positions to be truncated
 * @return false if one or more parameters is wrong (e.g. no categorical variable or
 * nPos >= length(variable)
 */
bool CMuArgCtrl::DoTruncate(long VarIndex, long nPos)
{
    int i, v = VarIndex - 1;
    int VarWidth, nCode, NewWidth;
    // too early?
    if (m_nvar == 0 || m_ntab == 0 || m_fname[0] == 0) {
	return false;
    }

    // wrong VarIndex
    if (v < 0 || v >= m_nvar || !m_var[v].IsCategorical) {
	return false;
    }
    if (m_var[v].Recode.DestCode != 0) {
        m_var[v].UndoRecode();
    }
    // initialize new recode
    if (!m_var[v].PrepareRecode()) {
        return false;
    }

    VarWidth = m_var[v].nPos;
    if (nPos >= VarWidth) {
	return false;
    }

    NewWidth = VarWidth - nPos;
    m_var[v].Recode.sCode.clear();

    nCode = m_var[v].nCode - m_var[v].nMissing;
    std::string newcode;

    for (i = 0; i < nCode; i++) {
        //newcode = m_var[v].sCode[i].Left(NewWidth);
        newcode = m_var[v].sCode[i].substr(0,NewWidth);
	int n = m_var[v].AddRecode(newcode.c_str());
	assert(n >= 0 && n < m_var[v].nCode);
	m_var[v].Recode.DestCode[i] = n;
    }

    m_var[v].MakeRecodelistEqualWidth("", "");

    m_var[v].Recode.Missing1 = m_var[v].Missing1;
    m_var[v].Recode.Missing2 = m_var[v].Missing2;
    m_var[v].Recode.nMissing = m_var[v].nMissing;
    m_var[v].HasRecode = true;

    //m_var[v].Recode.sCode.Add(m_var[v].Recode.Missing1);
    m_var[v].Recode.sCode.push_back(m_var[v].Recode.Missing1);
    m_var[v].Recode.DestCode[nCode] = m_var[v].Recode.sCode.size() - 1;
    if (m_var[v].nMissing == 2) {
        m_var[v].Recode.sCode.push_back(m_var[v].Recode.Missing2);
	m_var[v].Recode.DestCode[nCode + 1] = m_var[v].Recode.sCode.size() - 1;
    }

    m_var[v].Recode.nCode = m_var[v].Recode.sCode.size();

    return true;
}

/**
 * Re-calculates the (sub)tables, e.g., because of a recoding
 * Results can be retrieved using 
 * UnsafeVariable
 * UnsafeVariablePrepare
 * UnsafeVariableCodes
 * UnsafeVariableClose
 * @return always true
 */
bool CMuArgCtrl::ApplyRecode()
{
    ComputeRecodeTables();
    ComputeSubTableList();
    return true;
}

bool CMuArgCtrl::ComputeRecodeTables()
{
    int i, d, nRecodes = 0;

    // too early?
    if (m_nvar == 0 || m_ntab == 0 || m_fname[0] == 0) {
        return false;
    }

    // compute number of variables with a recode
    for (i = 0; i < m_nvar; i++) {
	if (m_var[i].HasRecode) nRecodes++;
    }

    if (nRecodes == 0) { // nothing to do, why do you call me?
	return false;
    }

    for (i = 0; i < m_ntab; i++) {
	int nDim = m_tab[i].nDim;
	for (d = 0; d < nDim; d++) {
            // a base table with at least one recode?
            if (m_var[m_tab[i].Varnr[d]].HasRecode) {
                break;
            }
	}
	if (d == nDim) continue; // table has no recodes
	m_tab[i + m_ntab].FreeRecodedTable();

	CTable dsttab = m_tab[i];
	dsttab.Cell = 0;
	dsttab.nCell =0;
	dsttab.BIRCell =0;
	for (d = 0; d < nDim; d++) {
            if (m_var[m_tab[i].Varnr[d]].HasRecode) {
                dsttab.SizeDim[d] = m_var[m_tab[i].Varnr[d] ].Recode.nCode;
            }
	}

	// compute and save recoded table
	ComputeRecodeTable(m_tab[i], dsttab);
	m_tab[m_ntab + i] = dsttab; // save for later use
	m_tab[i].HasRecode = true;
	m_tab[m_ntab + i].HasRecode = false;
    }

    return true;
}

bool CMuArgCtrl::ComputeRecodeTable(CTable & srctab, CTable & dsttab)
{
    if (!dsttab.PrepareTable()) {
        return false;
    }
    ComputeRecodeTableCells(srctab, dsttab, 0, 0, 0);
    return true;
}

void CMuArgCtrl::ComputeRecodeTableCells(CTable & srctab, CTable & dsttab, int niv, int iCellSrc, int iCellDst)
{
    int i, desti, nDim = srctab.nDim;

    if (niv == nDim) {
        assert(iCellSrc >= 0 && iCellSrc < srctab.nCell);
	assert(iCellDst >= 0 && iCellDst < dsttab.nCell);
	if (iCellDst == 4088)	{
            long Ramya = 0;
	}
	dsttab.Cell[iCellDst] += srctab.Cell[iCellSrc];
	if (dsttab.IsBIR) {
            dsttab.BIRCell[iCellDst] += srctab.BIRCell[iCellSrc];
	}
	return;
    }

    int iVar = srctab.Varnr[niv];
    int n = srctab.SizeDim[niv];

    bool HasRecode = m_var[iVar].HasRecode;

    for (i = 0; i < n; i++) {
        if (HasRecode) {
            desti = m_var[iVar].Recode.DestCode[i];
            assert(desti >= 0 && desti < m_var[iVar].Recode.nCode);
	}
	else {
            desti = i;
	}
    
        ComputeRecodeTableCells(srctab, dsttab, niv + 1, iCellSrc * srctab.SizeDim[niv] + i, iCellDst * dsttab.SizeDim[niv] + desti);
    }
}


bool CMuArgCtrl::WriteVariablesInFile(std::string FileNameMicro, std::string FileNameOut, long nVar, long *VarIndexes, std::string seperator, long *ErrorCode)
{
    std::string sFileNameMicro, sFileNameOut, sseperator;
    sFileNameMicro = FileNameMicro;
    sFileNameOut = FileNameOut;
    sseperator = seperator;
    FILE *fd, *fdout;
    char str[MAXRECORDLENGTH];
    int i, length;
    if (m_nvar == 0) {
	*ErrorCode = NOVARIABLES;
	return false;
    }

    fd = fopen(sFileNameMicro.c_str(), "r");
    if (fd == 0) {
        *ErrorCode = FILENOTFOUND;
	return false;
    }

    fdout = fopen(sFileNameOut.c_str(), "w");

    fseek(fd, 0, SEEK_END);
    m_fSize = ftell(fd);
    rewind(fd);

    str[0] = 0;
    fgets((char *)str, MAXRECORDLENGTH, fd);
    if (str[0] == 0) {
        *ErrorCode = EMPTYFILE;
	fclose(fd);fclose(fdout);
	return false;
    }

    length = strlen((char *)str) - 1;
    while (length > 0 && str[length] < ' ') length--;
    m_fixedlength = length + 1;
    if (length == 0) {
        *ErrorCode = EMPTYFILE; // first record empty
	fclose(fd); fclose(fdout);
	return false;
    }

    // record length oke?
    for (i = 0; i < m_nvar; i++) {
        if (m_var[i].bPos + m_var[i].nPos > m_fixedlength) {
            *ErrorCode = RECORDTOOSHORT;
            fclose(fd);fclose(fdout);
            return false;
	}
    }

    rewind(fd);
    int recnr = 0;
    while (!feof(fd) ) {
        int res;
	res = ReadMicroRecord(fd, str);
	switch (res) {
            case INFILE_ERROR:
                recnr++;
		*ErrorCode = WRONGLENGTH;
		fclose(fd);fclose(fdout);
		return false;
            case  INFILE_EOF:
		fclose(fd);
		fclose(fdout);
		return true;
            case  INFILE_OKE:
		recnr++;
		if (recnr % FIREPROGRESS == 0) {
                    FireUpdateProgress((int)(ftell(fd) * 100.0 / m_fSize));  // for progressbar in container
		}
		if (recnr > 1) {
                    fprintf(fdout,"\n");
		}

		if (!WriteVariablesFromMicroRecord(str, fdout, VarIndexes, nVar,sseperator)) {
                    *ErrorCode = WRONGRECORD;
                    fclose(fd);fclose(fdout);
                    return false;
		}
		break;
        }
    }

    fclose(fd);
    fclose(fdout);
    return true;
}

bool CMuArgCtrl::WriteVariablesFromMicroRecord(char *str, FILE *fdout, long *VarIndexes, long nVar, std::string seperator)
{
    // change this with respect to empty codes
    {
        int i, bp, ap, v;
	long lVarIndex;
	char code[MAXCODEWIDTH];
	std::string stemp;
	CVariable *var;
	std::string tempcode;
        
	for (i=0; i <nVar; i++)  {
            //lVarIndex = VarIndexes[i+1];
            lVarIndex = VarIndexes[i];
            if (lVarIndex-1 <0 ) {
                return false;
            }
            var = &(m_var[lVarIndex-1]);
            if (var->IsCategorical || var->IsNumeric) {
                if (m_InFileIsFixedFormat)  {
                    bp = var->bPos;         // startposition
                    ap = var->nPos;         // number of positions
                    strncpy(code, (const char *)&str[bp], ap); // get code from record
                    code[ap] = 0;
		}
		else {
                    ap = var->nPos;         // number of positions
//                  if (ReadVariableFreeFormat(str,i,&(tempcode))) { Dit geeft altijd de eerste variabele AHNL 04-01-2006
                    v = lVarIndex-1;
                    if (ReadVariableFreeFormat(str,v,&(tempcode))) {
                        strcpy(code,tempcode.c_str());
			code[ap] = 0;
                    }
		}
		stemp = code;
		stemp = trimleft(stemp);
		stemp = trimright(stemp);
		// now write code before code write seperator
		if (i > 0) {
                    fprintf(fdout, "%s", seperator.c_str());
		}
		fprintf(fdout, "%s", stemp.c_str());
            }
            else {
                continue;
            }
	}
//	fprintf(fdout,"\n");
	return true;
    }
}

bool CMuArgCtrl::SetInFileInfo(bool IsFixedFormat, std::string Seperator, bool IgnoreFirstLine)
{
    if (IsFixedFormat) {
	m_InFileIsFixedFormat = true;
    }
    else {
	m_InFileIsFixedFormat = false;
    }
    m_InFileSeperator = Seperator;

    if (!m_InFileIsFixedFormat) {
        if (IgnoreFirstLine) {
            m_IgnoreFirstLine = true;
	}
    }
    
    return true;
}

long CMuArgCtrl::NumberofRecords()
{
    return m_NumberofRecs;
}

long CMuArgCtrl::NumberOfHouseholds()
{
    return m_lNumberOfHH;
}


bool CMuArgCtrl::SetOutFileInfo(bool IsFixedFormat, std::string Seperator, std::string FirstLine, bool StringsInQuotes)
{
    if (IsFixedFormat) {
    	m_OutFileIsFixedFormat = true;
    }
    else {
	m_OutFileIsFixedFormat = false;
    }
    m_OutFileSeperator = Seperator;
    m_FirstLine = FirstLine;
    if (StringsInQuotes) {
	m_StringsInQuotes = true;
    }
    else {
	m_StringsInQuotes = false;
    }

    return true;
}


bool CMuArgCtrl::SetNumberOfChangeFiles(long nFiles)
{
    if (nFiles < 0 ) {
	return false;
    }
    
    m_nChangeFiles = nFiles;

    m_ChangeFiles = new CChSafeVarInfo [ nFiles];
    return true;
}

/**
 * Sets the value above which a combination is unsafe
 * @param TabIndex  1, 2, ..., n_tab index of table
 * @param Threshold Value above which a variable combination is unsafe
 * @param nUnsafe   Number of unsafe combinations with this Threshold
 * @return false if TableIndex is incorrect
 */
bool CMuArgCtrl::SetBirThreshold(long TabIndex, double Threshold, long *nUnsafe)
{
    double Ksi;
    int i = TabIndex - 1;

    if (i < 0 || i >= m_ntab || !m_tab[i].IsBIR){
	return false;
    }

    CTable *t = &(m_tab[i]);
    if (t->HasRecode) {
	t = &(m_tab[m_ntab + i]); // take the recoded one
    }

    t->BIRThreshold = Threshold;
    // set also in subtables
    for (int j = 0; j < m_nUC; j++) {
        if (m_UCList[j].TabNr == i) {
            m_UCList[j].table.BIRThreshold = Threshold;
	}
    }

    // compute nUC
    t->BIRUnsafe = 0;
    int DimNr[MAXDIM];
    AddMissingTable(*t, 0, DimNr, false, BIR_UNSAFE, &Ksi, 0);
    *nUnsafe = t->BIRUnsafe;

    return true;
}

/**
 * Sets for a given numerical variable the code to be used to replace all values above 
 * a certain value. E.g., all values above 600,000 receive the text ">=600,000"
 * @param VarIndex  Index of the variable
 * @param TopLevel  values >= TopLevel are to be replaced
 * @param TopString Code used for replacement
 * @param TopUndo   False when setting the topcode, true when undoing it
 * @return false if VarIndex is wrong
 */
bool CMuArgCtrl::SetCodingTop(long VarIndex, double TopLevel, std::string TopString, bool TopUndo)
{
    int v = VarIndex - 1;

    if (v < 0 || v >= m_nvar || !m_var[v].IsNumeric) {
	return false;
    }

    m_var[v].HasCodingTop = (TopUndo == 0);
    m_var[v].TopLevel = TopLevel;
    m_var[v].TopString = TopString;

    return true;
}

/**
 * Sets for a given numerical variable the code to be used to replace all values  
 * below a certain value. E.g., all values below 100,000 receive the text "<=100,000"
 * @param VarIndex      Index of the variable
 * @param BottomLevel   values <= BottomLevel are to be replaced
 * @param BottomString  Code used for replacement
 * @param BottomUndo    False when setting the bottomcode, true when undoing it
 * @return false if VarIndex is wrong
 */
bool CMuArgCtrl::SetCodingBottom(long VarIndex, double BottomLevel, std::string BottomString, bool BottomUndo)
{
    int v = VarIndex - 1;

    if (v < 0 || v >= m_nvar || !m_var[v].IsNumeric){
	return false;
    }

    m_var[v].HasCodingBottom = (BottomUndo == 0);
    m_var[v].BottomLevel = BottomLevel;
    m_var[v].BottomString = BottomString;

    return true;
}

/**
 * Gives for a numerical variable the minimum and maximum value that occurs in the
 * input file
 * @param VarIndex  Index of the variable
 * @param Min       Minimum value
 * @param Max       Maximum value
 * @return false if parameter VarIndex is wrong
 */
bool CMuArgCtrl::GetMinMaxValue(long VarIndex, double *Min, double *Max)
{
    int v = VarIndex - 1;

    if (v < 0 || v >= m_nvar || !m_var[v].IsNumeric) {
	return false;
    }

    *Min = m_var[v].MinValue;
    *Max = m_var[v].MaxValue;

    return true;
}

/**
 * Completes the specification of SetPramValue
 * @param VarIndex  Index of the variable
 * @return false if VarIndex is wrong and/or not all codes are set
 */
bool CMuArgCtrl::ClosePramVar(long VarIndex)
{
    int v = VarIndex - 1, i, n;

    if (v != m_PramVarIndex)	{
	return false;
    }

    n = m_var[v].PramValue.size();
    for (i = 0; i < n; i++) {
        if (m_var[v].PramValue[i] == PRAM_NOT_NA) { // not filled?
            return false;
	}
    }

    // Variable is now ready to be used for PRAM
    return true;
}

/**
 * Specifies on which variable the following SetPramValue calls apply.
 * After these calls, the function ClosePramVar is to be called.
 * This function checks if all codes of the pramvariable have received a value.
 * @param VarIndex  Index of the variable
 * @param BandWidth Bandwidth (-1 of no bandwith, 1, 2, ... otherwise)
 * @param Undo      False to apply, true to undo
 * @return 
 */
bool CMuArgCtrl::SetPramVar(long VarIndex, long BandWidth, bool Undo)
{
    bool bUndo;
    if (Undo){
        bUndo = true;
    }
    else{
	bUndo = false;
    }
    
    int v = VarIndex  - 1, c, n;

    if (v < 0 || v >= m_nvar || !m_var[v].IsCategorical) {
	return false;
    }

    m_var[v].HasPram = (bUndo == false);
    if (Undo)  {
	if (m_PramVarIndex >= 0) {
            m_var[v].Entropy = -1;
            m_var[v].PramBandWidth = -1;
	}
	m_PramVarIndex = -1;           // disable pram variable
	return true;
    }

    m_PramVarIndex = v;              // remember index variable
    m_var[v].PramValue.clear();  // clean a previous one

    n = m_var[v].GetnCodes(false);

    //m_var[v].PramValue.SetSize(n, 50);  // initialize
    m_var[v].PramValue.resize(n, PRAM_NOT_NA);  // initialize

    //for (c = 0; c < n; c++) {
    //    m_var[v].PramValue[c] = PRAM_NOT_NA;  // initialize percentages with na
    //}

    if (BandWidth != -1 && BandWidth < 1) {  // -1, 1, 2, 3, ...
	return false;
    }

    m_var[v].PramBandWidth = BandWidth;
    return true;
}

/**
 * Sets the PRAM-percentage for a code. This percentage is the probability that 
 * the code does not change. If the code is changed, it is changed into one of 
 * the other codes , each with probability (100 - Value)/n where n equals
 * number of codes - 1  in case bandwidth is not applicable
 * 2*bandwidth          in case bandwidth is applicable
 * If bandwidth is applicable codes are only changed within a certain distance 
 * from the original code. A code of 13 for index to be changed, with 
 * bandwidth = 3 can only be changed to 10, 11, 12, 14, 15 or 16, each with 
 * equal probability.
 * A missing will never be changed.
 * Pram-percentage ranges from 0 to 100 (both inclusive)
 * @param CodeIndex Index of Code
 * @param Value     Pram-percentage of not-changing
 * @return false if CodeIndex is wrong, Value < 0, Value > 100
 */
bool CMuArgCtrl::SetPramValue(long CodeIndex, long Value)
{
    int c = CodeIndex - 1, n, v = m_PramVarIndex;

    if (v < 0 || v >= m_nvar)	{
	return false;
    }
    if (Value < 0 || Value > 100)	{
	return false;
    }

    n = m_var[v].GetnCodes(false);
    if (c < 0 || c >= n)	{
	return false;
    }

    m_var[v].PramValue.at(c) = Value;
    return true;
}

/**
 * Gives for a certain number of dimensions the properties of a table.
 * @param nDim      Number of dimensions
 * @param Index     Sequence number
 * @param BaseTable Is a base table (not a subtable)
 * @param nUC       Number of UCs
 * @param VarList   nDim variable indices of the (sub) table
 * @return true if table with requested index is found. 
 *          If for Index==1 false is returned, there are no tables with that number of dimensions
 */
bool CMuArgCtrl::GetTableUC(long nDim, long Index, bool *BaseTable, long *nUC, long *VarList)
{
    int i, d = 0;
    CUCList uc;

    if (m_nvar == 0 || m_ntab == 0 || m_fname[0] == 0) {
	return false;
    }

    for (i = 0; i < m_nUC; i++) {
	uc = m_UCList[i];
	if (!uc.biggestThreshold) continue;
	if (uc.nDim == nDim) {
            if (++d == Index) { // that's the one
                *BaseTable = (nDim == m_tab[uc.TabNr].nDim);
		*nUC = uc.nUC;
		for (int j = 0; j < nDim; j++) {
                    VarList[j] = uc.Varnr[j] + 1;
		}
/*#ifdef _DEBUGG
        TRACE("nDim=%d, Index=%3d, Base=%d, nUc=%d VarIndices:", nDim, Index, *BaseTable, *nUC);
        for (j = 0; j < nDim; j++) {
          TRACE("%2d", VarList[j]);
        }
        TRACE("\n");
#endif // _DEBUGG */
		return true;
            }
	}
    }
//    *pVal = VARIANT_FALSE;
    return false;
}

bool CMuArgCtrl::GetErrorString(long errorCode, const char ** errorString) {
    std::string str("Unknown error");
    switch (errorCode) {
        case FILENOTFOUND:
            str = "File not found";
            break;
        case CANTOPENFILE:
            str = "Cannot open file";
            break;
        case EMPTYFILE:
            str = "File is empty";
            break;
        case WRONGLENGTH:
            str = "Record has wrong length";
            break;
        case RECORDTOOSHORT:
            str = "Record is too short";
            break;
        case WRONGRECORD:
            str = "Error in record";
            break;
        case NOVARIABLES:
            str = "No variables have been defined";
            break;
        case NOTABLES:
            str = "No tables have been defined";
            break;
        case NOTENOUGHMEMORY:
            str = "There is not enough memory";
            break;
        case NOTABLEMEMORY:
            str = "Not enough memory for table";
            break;
        case NODATAFILE:
            str = "No data file specified";
            break;
        case E_HARD:
            str = "Syntax error";
            break;
        case E_LENGTHWRONG:
            str = "Wrong code length";
            break;
        case E_VARINDEXWRONG:
            str = "Wrong variable index";
            break;
        case E_RANGEWRONG:
            str = "Invalid range";
            break;
        case E_NOVARTABDATA:
            str = "Not all metadata specified";
            break;
        case E_EMPTYSPEC:
            str = "Empty specification";
            break;
    }
    *errorString = str.c_str();
    return true;
}

/**
 * Gives for a categorical variable the code-string and PRAM-percentage
 * @param VarIndex  Index of the variable
 * @param CodeIndex Index of the code
 * @param Code      Alphanumeric value of the code
 * @param PramPerc  Percentage to be used in PRAM, -1 if not applicable
 * @return false if parameter VarIndex and/or CodeIndex is wrong
 */
bool CMuArgCtrl::GetVarCode(long VarIndex, long CodeIndex, const char **Code, long *PramPerc)
{
    int v = VarIndex - 1, c = CodeIndex - 1, n;

    if (v < 0 || v >= m_nvar || !m_var[v].IsCategorical)	{
	return false;
    }

    n = m_var[v].GetnCodes(false);
    if (c < 0 || c >= n)	{
	return false;
    }

    if (m_var[v].HasRecode) {
        *Code = m_var[v].Recode.sCode[c].c_str();
    } else {
	*Code = m_var[v].sCode[c].c_str();
    }

    if (m_var[v].HasPram) {
	*PramPerc = m_var[v].PramValue[c];
    } else {
	*PramPerc = -1;
    }

    return true;
}

/**
 * Provides for a variable the position in the safe file, the number of times the variable is suppressed and the entropy
 * @param VarIndex      1, 2, ..., n_var index of variable
 * @param StartPos      Start position in safe file
 * @param nPos          Number of positions
 * @param nSuppress     Number of times suppressed
 * @param Entropy       Entropy of variable, -1 if no entropy
 * @param BandWidth     Bandwidth entropy of variable, -1 if no bandwidth entropy
 * @param Missing1      Alphanumeric value Missing1, blank if not applicable
 * @param Missing2      Alphanumeric value Missing2, blank if not applicable
 * @param NofCodes      Number of codes in safe file (could differ from original, due to Recoding)
 * @param NofMissing    Number of Missing codes in safe file
 * @return false if error
 */
bool CMuArgCtrl::GetVarProperties(long VarIndex, long *StartPos, long *nPos, long *nSuppress, double *Entropy, long *BandWidth, const char **Missing1, const char **Missing2, long *NofCodes, long *NofMissing)
{
    int i, v = VarIndex - 1;

    if (v < 0 || v >= m_nvar)	{
	return false;
    }

    // find var in outputvarlist
    for (i = 0; i < m_nvarpos; i++) {
	if (m_varlist[i].VarIndex == v) break;
    }
    if (i == m_nvarpos)	{  // hm, not found
	return false;
    }
	
    *StartPos = m_varlist[i].d_bpos + 1;
    *nPos = m_varlist[i].d_npos;  // ?? kan 0 zijn bij een verdwenen HHIdent
    *nSuppress = m_var[v].nSuppress;
    *Entropy = m_var[v].Entropy;  // in case na: -1
    if (m_var[v].IsCategorical) {
	*NofCodes = m_var[v].GetnCodes(false);
	*NofMissing = m_var[v].GetnCodes(true) - *NofCodes;
    }
    else {
	*NofCodes = 0;
	*NofMissing = 0;
    }

    *Missing1 = ""; // = 0
    *Missing2 = ""; // = 0
    if (m_var[v].IsCategorical) {
	if (m_var[v].HasRecode) {
            int c = m_var[v].Recode.nCode - m_var[v].Recode.nMissing;
		*Missing1 = m_var[v].Recode.sCode[c].c_str();
		if (m_var[v].Recode.nMissing > 1) {
                    *Missing2 = m_var[v].Recode.sCode[c + 1].c_str();
		}
	}
	else {
            int c = m_var[v].nCode - m_var[v].nMissing;
            *Missing1 = m_var[v].sCode[c].c_str();
            if (m_var[v].nMissing > 1) {
		*Missing2 = m_var[v].sCode[c + 1].c_str();
            }
	}
    }

    *BandWidth = m_var[v].PramBandWidth; // in case na: -1
    return true;
}


bool CMuArgCtrl::SetChangeFile(long FileIndex, std::string FileName, long nVar, long *VarIndex, std::string FileSeperator)
{
    long lFileIndex;
    lFileIndex = FileIndex -1;
    std::string sFileName;
    sFileName = FileName;
    std::string sSeperator;
    sSeperator = FileSeperator;
    long i;

    for (i=0; i<nVar; i++) {
	VarIndex[i] = VarIndex[i]-1;
    }

    //Number of files not set
    if (m_nChangeFiles < 0) {
	return false;
    }

    // Non existing file number
    if ((lFileIndex < 0) || (lFileIndex > m_nChangeFiles) )  {
	return false;
    }
    // already set
    if (strcmp(m_ChangeFiles[lFileIndex].GetFileName().c_str(), "") != 0) {
	return false;
    }

    m_ChangeFiles[lFileIndex].SetFileName(sFileName);
    m_ChangeFiles[lFileIndex].SetNumberVar(nVar);
    m_ChangeFiles[lFileIndex].SetVarIndex(VarIndex);
    m_ChangeFiles[lFileIndex].SetSeperator(sSeperator);

    return true;
}

/**
 * Sets for a numerical variable the rounding base. Rounding base can contain 
 * decimals, e.g. "2.5". In that case give as rounding base 2.5 and nDec=1.
 * @param VarIndex  Index of the variable
 * @param RoundBase Rounding base
 * @param nDec      Number of decimals in rounding base
 * @param Undo      False when setting the rounding base, true when undoing it
 * @return false if VarIndex is wrong, RounBase <= 0, nDec < 0
 */
bool CMuArgCtrl::SetRound(long VarIndex, double RoundBase, long nDec, bool Undo)
{
    int v = VarIndex - 1;
    if (v < 0 || v >= m_nvar || !m_var[v].IsNumeric || m_var[v].IsCategorical || RoundBase <= 0 || nDec < 0) {
	return false;
    }

    m_var[v].HasRound = (Undo == false);
    m_var[v].RoundBase = RoundBase;
    m_var[v].RoundnDec = nDec;

    return true;
}

/**
 * Sets for a categorical variable the priority when applying local suppression,
 * i.e., when imputing "Missing1".
 * It is possible that for a certain unsafe combination of variables, more than
 * one candidate is eligible to be suppressed. When calling MakeFileSafe with the 
 * option "with prior", the variable with the lowest priority is suppressed. 
 * In case of equal priorities, the first variable is taken (with the smallest VarIndex)
 * @param VarIndex  Index of the variable
 * @param Priority  Value of the priority
 * @return false in case VarIndex is wrong
 */
bool CMuArgCtrl::SetSuppressPrior(long VarIndex, long Priority)
{
    int v = VarIndex - 1;
    if (v < 0 || v >= m_nvar || !m_var[v].IsCategorical) {
	return false;
    }

    m_var[v].Priority = Priority;
    return true;
}

/**
 * Sets the percentage to decrease or increase the value of a numeric variable. 
 * Variable will be changed by percentage, randomly selected from the interval 
 * (100 - WeightNoise, 100 + WeightNoise)
 * @param VarIndex      Index of the variable
 * @param WeightNoise   Percentage by which the value is reduced/increased
 * @param Undo          False when applying, true when undoing
 * @return false if VarIndex if wrong, WeightNoise <= 0, WeighthNoise > 100
 */
bool CMuArgCtrl::SetWeightNoise(long VarIndex, double WeightNoise, bool Undo)
{
    int v = VarIndex - 1;

    if (v < 0 || v >= m_nvar || !m_var[v].IsWeight) {
	return false;
    }

    if (WeightNoise <= 0.0 || WeightNoise > 100.0)	{
	return false;
    }

    m_var[v].HasWeightNoise = (Undo == false);
    m_var[v].WeightNoise = WeightNoise;
    return true;
}

/**
 * Invalidates the special processing options:
 * SetRound
 * SetCodingTop
 * SetCodingBottom
 * SetWeightNoise
 * SetPramVar
 * @return always true
 */
bool CMuArgCtrl::MakeFileSafeClearOptions()
{
    int i;

    for (i = 0; i < m_nvar; i++) {
        if (m_var[i].IsNumeric) {
            m_var[i].HasRound = false;
            m_var[i].HasCodingTop = false;
            m_var[i].HasCodingBottom = false;
            m_var[i].HasWeightNoise = false;
//          m_var[i].HasPram = false; //Alleen zinnig bij niet cat. variabelen
	}
    }
    return true;
}


bool CMuArgCtrl::ComputeBIRRateThreshold(long TabIndex, double MaxRisk, double *ReIdentRate)
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleState())
    int i = TabIndex - 1, DimNr[MAXDIM];

    if (i < 0 || i >= m_ntab)	{
	return false;
    }
    if (!m_tab[i].IsBIR)	{
	return false;
    }

    CTable *t = &(m_tab[i]);
    if (t->HasRecode) {
	t = &(m_tab[m_ntab + i]); // take the recoded one
    }

    AddMissingTable(*t, 0, DimNr, false, BIR_RATE_RISK, ReIdentRate, MaxRisk); // computes m_BIRMaxValue and m_BIRMinValue

    *ReIdentRate = *ReIdentRate / m_NumberofRecs;

    return true;
}

/**
 * Histogram gives information about the BaseIndividualRisk base. 
 * This is the natural logarithm of the result of that function.
 * @param TabIndex          1, 2, ..., n_tab Index of table
 * @param nClasses          Number of histogram classes
 * @param ClassLeftValue    Array of nClasses+1 doubles: lower limits of the classes + upper limit of last class
 * @param Ksi               Re-identification rate
 * @param Frequency         Array of nClasses longs: BIR number per class
 * @return false if TabIndex is wrong or some other silly error
 */
bool CMuArgCtrl::GetBIRHistogramData(long TabIndex, long nClasses, double *ClassLeftValue, double *Ksi, long *Frequency)
{
    int i = TabIndex - 1, k, DimNr[MAXDIM];

    if (i < 0 || i >= m_ntab)	{
	return false;
    }
    if (!m_tab[i].IsBIR)	{
	return false;
    }
    if (nClasses < 2)	{
	return false;
    }

    CTable *t = &(m_tab[i]);
    if (t->HasRecode) {
        t = &(m_tab[m_ntab + i]); // take the recoded one
    }

    // compute min and max risk
    t->BIRMaxValue = -DBL_MAX;
    t->BIRMinValue = DBL_MAX;
    t->BIRnClasses = nClasses;
    AddMissingTable(*t, 0, DimNr, false, BIR_MINMAX, Ksi, 0); // computes m_BIRMaxValue and m_BIRMinValue

    // compute class width
    t->BIRClassWidth= (t->BIRMaxValue - t->BIRMinValue) / nClasses;

    // compute classboundaries
    for (k = 0; k <= nClasses; k++) {
        ClassLeftValue[k] = t->BIRMinValue + k * t->BIRClassWidth;
    }

    // count frequency per class
    // first set on zero
    for (k = 0; k < nClasses; k++) {
        Frequency[k] = 0;
    }

    *Ksi = 0;
    AddMissingTable(*t, 0, DimNr, false, BIR_FREQ, Ksi, 0, Frequency); // computes frequencies
    *Ksi = *Ksi / m_NumberofRecs;
/*
#ifdef _DEBUGG
  { int i;
    TRACE("ClassBoundaries:\n");
    for (i = 0; i <= nClasses; i++) {
      TRACE("%2d. %.2f\n", i + 1, ClassLeftValue[i]);
    }
    TRACE("Frequencies:\n");
    for (i = 0; i < nClasses; i++) {
      TRACE("%2d. %5d\n", i + 1, Frequency[i]);
    }
  }
#endif _DEBUG
*/
    return true;
}

bool CMuArgCtrl::AddMissingTable(CTable & t, int niv, int *DimNr, bool HasMissing, int type, double *Ksi, double MaxRisk, long *Frequency)
{
    int i, nMissing, nCode;
    bool temp;
    if (niv == t.nDim) {
        long freq = t.Cell[t.GetCellNr(DimNr)];
	double weight, v, Logv;
	if (freq > 0) {
            AddMissing(t, DimNr, freq, weight, HasMissing); // all corresponding Missing(s) are added
            temp = BaseIndividualRisk(freq, weight, &v);
            assert(temp);
            if (v != 0) {
	  	// take log of it
                Logv = log(v);
		switch (type) {
                    case BIR_MINMAX:
			if (Logv > t.BIRMaxValue) t.BIRMaxValue = Logv;
			if (Logv < t.BIRMinValue) t.BIRMinValue = Logv;
			break;
                    case BIR_FREQ:
			assert(Logv <= t.BIRMaxValue && Logv >= t.BIRMinValue);
			{ 
                            int ci = (int) ((Logv - t.BIRMinValue) / t.BIRClassWidth);
                            if (ci == t.BIRnClasses) ci--;  // can happen in rare cases, due to rounding
                            assert(ci >= 0 && ci < t.BIRnClasses);
                            Frequency[ci] += freq;
			}
			// hier tellen voor de Re-indent.rate
			*Ksi = *Ksi + freq * v;
			break;
			case BIR_UNSAFE:
                            if (Logv >= t.BIRThreshold) {
                                t.BIRUnsafe += t.Cell[t.GetCellNr(DimNr)];
                            }
                            break;
			case BIR_RATE_RISK:
                            if (v > MaxRisk){
                                *Ksi = *Ksi + freq * MaxRisk;
                            }
                            else {
                                *Ksi = *Ksi + freq * v;
                            }
                            break;
		}
            }
	}
    }
    else {  // compute all Cells recursively
        if (m_var[t.Varnr[niv]].HasRecode) {
            nMissing = m_var[t.Varnr[niv]].Recode.nMissing;
            nCode = m_var[t.Varnr[niv]].Recode.nCode - nMissing;
	}
	else {
            nMissing = m_var[t.Varnr[niv]].nMissing;
            nCode = m_var[t.Varnr[niv]].nCode - nMissing;
	}
	for (i = 0; i < nCode; i++) { // valid codes
            DimNr[niv] = i;
            AddMissingTable(t, niv + 1, DimNr, HasMissing, type, Ksi, MaxRisk, Frequency);
	}
	DimNr[niv] = i; // Missing1
	AddMissingTable(t, niv + 1, DimNr, true, type, Ksi, MaxRisk, Frequency);
    }

    return true;
}


// DimNr:  array of indices for every dimension
// HasMissing: true if at least one is an index of a Missing Value
// Horrendous algorithm stay off it
int CMuArgCtrl::AddMissing(const CTable& tab, int *DimNr, long& freq, double& weight, bool HasMissing)
{
    int i, j, k, n, nDim;
    int dimnr[MAXDIM] = {-1}, nMissing[MAXDIM], vars[MAXDIM];
    CTable t = tab;  // make copy

    // for example a 3-dim table ABC, then do all 2 ^ 3 possibilities (M = missing)
    // ABC, ABM, AMC, AMM, MBC, MBM, MMC, MMM

    if (HasMissing) {  // use a subtable of tab, but which one?
        // Get valid variables
        int vi = 0;
	for (i = 0; i < tab.nDim; i++) {
            if (m_var[tab.Varnr[i]].HasRecode) {
                if (DimNr[i] == m_var[tab.Varnr[i]].Recode.nCode - m_var[tab.Varnr[i]].Recode.nMissing) { // Missing1
                    continue;
		}
            }
            else {
                if (DimNr[i] == m_var[tab.Varnr[i]].nCode - m_var[tab.Varnr[i]].nMissing) { // Missing1
                    continue;
		}
            }
            vars[vi++] = tab.Varnr[i];
	}

	if (vi == 0) return false; // all variables on Missing

	assert(vi > 0 && vi < tab.nDim);

	// now find table with this variables, table has vi dimensions
	t.nDim = 0;
	for (i = 0; i < m_nUC; i++) {
            if (m_UCList[i].nDim != vi) continue;
            for (j = 0; j < vi; j++) {
                if (vars[j] != m_UCList[i].Varnr[j]) {
                    break;
		}
            }
            if (j == vi && m_UCList[i].table.IsBIR) {  // table found!
            t = m_UCList[i].table;
            break;
            }
	}
	assert(t.nDim > 0);
    }

    nDim = t.nDim;
    n = (int) (pow(2, t.nDim) + .01);

    freq = 0;
    weight = 0;

    // now add Missings from table t
    for (i = 0; i < n; i++) {                       // for every combination of variables
        for (j = 1, k = 0; k < nDim; j <<= 1, k++) {  // shift bit one place to the left
            if (j & i) {                                // bit is on?
                if (m_var[t.Varnr[k]].HasRecode) {
                    nMissing[k] = m_var[t.Varnr[k]].Recode.nMissing;
                    dimnr[k] = m_var[t.Varnr[k]].Recode.nCode - nMissing[k]; // take Missing1 Recode
		}
                else {
                    nMissing[k] = m_var[t.Varnr[k]].nMissing;
                    dimnr[k] = m_var[t.Varnr[k]].nCode - nMissing[k]; // take Missing1
		}
            }
            else {
                if (HasMissing) {
                    // find variable index in source table with vars[k]
                    int d;
                    for (d = 0; d < tab.nDim; d++) {
                        if (vars[k] == tab.Varnr[d]) break;
                    }
                    assert(d < tab.nDim);
                    dimnr[k] = DimNr[d];  // take valid code subtable
		}
		else {
                    dimnr[k] = DimNr[k];  // take valid code source table, equal place of course
		}
		nMissing[k] = 0;
            }
	}
	assert(dimnr[0] >= 0);
	AddMissingCells(t, dimnr, nMissing, freq, weight);
    }
    return true;
}


// Missing1 and, if present, Missing2
void CMuArgCtrl::AddMissingCells(CTable& t, int *dimnr, int *nMissing, long& freq, double& weight)
{
    long CellNr;

    CellNr = t.GetCellNr(dimnr);
    freq += t.Cell[CellNr];
    weight += t.BIRCell[CellNr];
    // don't forget Missing2
    for (int i = 0; i < t.nDim; i++) {
        if (nMissing[i] == 2) {
            dimnr[i]++;
            CellNr = t.GetCellNr(dimnr);
            freq += t.Cell[CellNr];
            weight += t.BIRCell[CellNr];
            dimnr[i]--;
	}
    }
}

/**
 * Creates a safe file:
 *      If requested, calculates the entropy of categorical variables 
 *      Makes a record description of the output file
 *      If there are PRAM variables, the tables of the PRAM variables are ignored for local suppression
 *      Takes into account that if a household variable changes, it changes in
 *      every household record in the same way
 *      Calculates the best combination of variables that must be set to Missing, taking
 *      account of entropy, or weights, when creating a safe record
 * If both WithPrior and WithEntropy are false: nothing will be suppressed
 * @param FileName          Name of the safe file
 * @param WithPrior         Use priorities
 * @param WithEntropy       Use entropy
 * @param HHIdentOption     Option to use HHIdent (household identifier)
 *                          0: There is no household
 *                          1: Keep Household variables consistent and do not change HHIdent
 *                          2: Keep Household variables consistent and change HHIdent into a serial number
 *                          3: Keep Household variables consistent and remove HHIdent
 *                          Maintaining consistency means that if in at least one record in a household a Household variable is
 *                          changed into Missing, that variable is set to Missing in all other records of that household.
 * @param RandomizeOutput   Writes records in random order to the safe file. Only for fixed format data.
 * @param PrintBHR
 * @return false if something went wrong
 */
bool CMuArgCtrl::MakeFileSafe(std::string FileName, bool WithPrior, bool WithEntropy, long HHIdentOption, bool RandomizeOutput, bool PrintBHR)
{
    std::string sFileName;
    sFileName = FileName;
    FILE *fd_in, *fd_out;
    char str[MAXRECORDLENGTH];
    char orgstr[MAXRECORDLENGTH];
    int i, j, recnr, nRecHH = 1;
    int* InvolvedVar;
    bool bPrintBHR;
    if (PrintBHR)	{
    	if ((m_lNumBIRs <= 0)  && (m_lNumberOfHH <= 0)) {
            return false;
	}
	bPrintBHR = true;
    }
    else {
	bPrintBHR = false;
    }

    m_WriteRandom = RandomizeOutput;
    m_HHIdentOption = HHIdentOption;

    if (HHIdentOption != HHIDENT_NO) { // there are householdrecords
        if (m_HHIdentVar < 0) {          // no HHIdent specified
            return false;
	}
    }

    m_HHSeqNr = 0;

    InvolvedVar = new int[m_HHVars.size()];

    if (m_nvar == 0 || m_ntab == 0 || m_fname[0] == 0) {
	return false;
    }

    // seed randomgenerator
    srand( (unsigned) time(NULL));

    // zero m_nMissing in variables
    for (i = 0; i < m_nvar; i++) {
        m_var[i].nSuppress = 0;
    }

    m_WithPriority = WithPrior;
    m_WithEntropy  = WithEntropy;

    // open input
    fd_in = fopen(m_fname, "r");
    if (fd_in == 0) {
	return false;
    }

    // open output
    char sWriteType[10];
    if (m_WriteRandom) {
        strcpy(sWriteType, "wb");
	// compute m_WriteRecnr and m_WriteCoPrime
	if (m_nRecFile < 10) {
            m_WriteCoPrime = 2;
	}
	else {
            srand(time(NULL));
            m_WriteCoPrime = (rand() % (m_nRecFile / 2) );
            if (m_WriteCoPrime < 2) m_WriteCoPrime = 2;
	}
	// 2 <= coprime < n_rec
        while ( GGD(m_nRecFile, m_WriteCoPrime) != 1) m_WriteCoPrime++;
	m_WriteRecNr = rand() % m_WriteCoPrime;
	m_WriteRecNr = m_WriteRecNr % m_nRecFile; // to be quite sure
    }
    else {
        strcpy(sWriteType, "w");
    }

    fd_out = fopen(sFileName.c_str(), sWriteType);
    if (fd_out == 0) {
	return false;
    }

    if (m_OutFileIsFixedFormat) {
    	MakeRecordDescription( HHIdentOption); // If FileName empty: no *.rda written
    }
    else {
        MakeFreeRecordDescription(HHIdentOption);
    }

    if (WithEntropy) {
        for (i = 0; i < m_nvar; i++) {
            if (m_var[i].IsCategorical) {
                DoEntropy(i, m_var[i].Entropy);
				/*
#ifdef _DEBUGG
        TRACE("Entropy var %c: %f\n", i + 'A', m_var[i].Entropy);
#endif // _DEBUG */

            }
	}
    }

// compute in n_UCList combinations with PRAM
    for (i = 0; i < m_nUC; i++) {
        if (!m_UCList[i].biggestThreshold) continue;  // table irrelevant
        for (j = 0; j < m_UCList[i].nDim; j++) {
            if (m_var[m_UCList[i].Varnr[j]].HasPram) {
                break;
            }
        }
        m_UCList[i].HasPram = (j != m_UCList[i].nDim);  // a table with a pram variable, make irrelevant
    }


/*
#ifdef SHOWUNSAFE
  fd_test = fopen("unsafe.txt", "w");
#endif  // SHOWUNSAFE
*/
	// zo nodig alle records van een huishouden bijeenhouden
  // om de huishoudvariabelen identiek te behandelen

    m_nUnsafe = 0;
    recnr = 0;
    int ReadCode;

    if (HHIdentOption == HHIDENT_NO) { // no householdrecords
	while (1) {
            if (ReadCode = ReadMicroRecord(fd_in, str), ReadCode != INFILE_OKE) {
                assert(ReadCode != INFILE_ERROR);
                break;  // error (should not be possible) or eof
            }
            recnr++;
            if (recnr % FIREPROGRESS == 0) {
                FireUpdateProgress((int)(recnr * 100.0 / m_nRecFile) );  // for progressbar in container
            }

            if ((!m_InFileIsFixedFormat)&&(m_IgnoreFirstLine)&&(recnr == 1)) {
                if (!m_FirstLine.empty()) {
                    fprintf(fd_out, "%s\n", m_FirstLine.c_str());
                }
                continue;
            }
            strcpy((char *)orgstr,(char *)str);
            if (!MakeRecordSafe(str, 0, recnr, 1, -1) ) { // Households are not a problem
                goto error;
            }
                // now replace the string with some stuff
            WriteRecord(fd_out, str, HHIdentOption, recnr, false, 0, bPrintBHR,orgstr);
        }
    }
    else { // HHvar is activated, varnr in m_HHIdentVar
        char CurrHH[MAXCODEWIDTH];           // current household ident
	char PrevHH[MAXCODEWIDTH] = "";      // previous houshold ident
	long RecPos, StartHHPos = 0;
	int bPos = m_var[m_HHIdentVar].bPos;
	int nPos = m_var[m_HHIdentVar].nPos;
	int FirstRecHH = true, nRecHH;

	CurrHH[nPos] = 0;

        while (1) {
            RecPos = ftell(fd_in);
            if (ReadCode = ReadMicroRecord(fd_in, str), ReadCode != INFILE_OKE) {
                assert(ReadCode != INFILE_ERROR);
		if (ReadCode == INFILE_ERROR) {
                    break;  // error, should not be possible
		}
            }
            // find out if it is a neww HH except for first record. then print it out
            recnr++;
            // This has to change to take into account
            // Free Format Files
            if ((!m_InFileIsFixedFormat)&&(m_IgnoreFirstLine)&&(recnr == 1)) {
                if (!m_FirstLine.empty()) {
                    fprintf(fd_out, "%s\n", m_FirstLine.c_str());
		}
		continue;
            }

            if (recnr % FIREPROGRESS == 0) {
                FireUpdateProgress((int)(recnr * 100.0 / m_nRecFile) );  // for progressbar in container
            }
            strncpy(CurrHH, (char *) (&str[bPos]), nPos);
            if (ReadCode == INFILE_EOF || strncmp(CurrHH, PrevHH, nPos) != 0) {  // new HH
                if (PrevHH[0] != 0) {   // do now the complete HH
                    long TempRecPos = ftell(fd_in);      // remember current position
                    m_HHSeqNr++;
                    //here you have to save the file
                    DoCompleteHH(fd_in, fd_out, StartHHPos, nRecHH, InvolvedVar, recnr, HHIdentOption, /*HHcount-1*/ m_HHSeqNr-1, bPrintBHR);   // do the HH
                    fseek(fd_in, TempRecPos, SEEK_SET);  // set at current position
                    FirstRecHH = true;
                }
		strncpy(PrevHH, CurrHH, nPos);
		StartHHPos = RecPos;
		for (int v = 0; v < m_HHVars.size(); v++) {
                    InvolvedVar[v] = false;
		}
            }
            if (ReadCode == INFILE_EOF) break;

            if (FirstRecHH) {
                long cpos = ftell(fd_in);
		fseek(fd_in, StartHHPos, SEEK_SET);
		nRecHH = ComputeRecHH(fd_in, bPos, nPos);
		assert(nRecHH > 0);
		fseek(fd_in, cpos, SEEK_SET);
		FirstRecHH = false;
            }

            if (!MakeRecordSafe(str, 0, recnr, nRecHH,m_HHSeqNr) ) {
                goto error;
            }
            // HHVars involved?
            for (i = 0; i < m_HHVars.size(); i++) {
                if (m_var[m_HHVars[i] ].SetMissing) {
                    InvolvedVar[i] = true;
		}
            }
	}
    }

    //TRACE("Total Unsafe: %d\n", m_nUnsafe);

#ifdef SHOWUNSAFE
  fclose(fd_test);
#endif // SHOWUNSAFE

    fclose(fd_in);
    fclose(fd_out);
    delete [] InvolvedVar;
    FireUpdateProgress(100);  // for progressbar in container

    return true;

error:
    fclose(fd_in);
    fclose(fd_out);
    delete [] InvolvedVar;

    return false;
}

// compute Biggest Common Divisor
int CMuArgCtrl::GGD(int a, int b)
{ 
    if (b == 0) return a;
    else        return ( GGD(b, a % b) );
}

bool CMuArgCtrl::MakeRecordDescription(long HHIdentOption)
{
    int v, pos, i;
    //CStringArray rec;
    std::vector<std::string> rec;
    std::string s;
    char str[100];

    // compute used positions in source record
    for (v = 0; v < m_nvar; v++) {
        sprintf(str,"%5d %5d %5d", m_var[v].bPos, m_var[v].nPos, v);
	rec.push_back(str);
    }

    QuickSortStringArray(rec, 0, rec.size() - 1);

    // compute unused positions in source record
    pos = 0;
    {
        int bp, np, n = m_nvar;
	for (v = 0; v < n; v++) {
            s = rec[v];
            strcpy(str, s.c_str());
            bp = atoi(&str[0]);
            np = atoi(&str[6]);
            assert(bp >= pos);
            if (bp < pos) {
                return false;  // overlapping fields
            }

            if (bp > pos) {
                sprintf(str,"%5d %5d %5d", pos, bp - pos, -1);
		rec.insert(rec.begin()+v, str);
		v++;
		n++;
            }
            pos = bp + np;
	}
	if (pos < m_fixedlength) {  // last piece
            sprintf(str,"%5d %5d %5d", pos, m_fixedlength - pos, -1);
            rec.push_back(str);
	}
    }

    // number of fields in outputrecord
    m_nvarpos = rec.size();

    if (m_varlist != 0) {
        delete [] m_varlist;
    }
    m_varlist = new CVarList[m_nvarpos];
    if (m_varlist == 0) {
        return false;
    }

    // compute destination positions
    {
        int pos = 0, np, iVar;

	for (v = 0; v < m_nvarpos; v++) {
            s = rec[v];
            strcpy(str, s.c_str());
            m_varlist[v].s_bpos = atoi(&str[0]);
            m_varlist[v].s_npos = atoi(&str[6]);
            m_varlist[v].d_bpos = pos;
            iVar = atoi(&str[12]);
            m_varlist[v].VarIndex = iVar;
            if (iVar >= 0) {
                if (iVar >= 0 && iVar < m_nvar)	{
                    np = m_var[iVar].ComputeWidth(atoi(&str[6]), HHIdentOption, m_nRecFile);
                    m_varlist[v].d_npos = np;
		}
            }
            else {
                np = m_varlist[v].s_npos;
		m_varlist[v].d_npos = np;
            }
            pos += np;
	}
    }
/*
#ifdef _DEBUGG
  {
    for (v = 0; v < m_nvarpos; v++) {
      TRACE("%2d. %5d %2d %5d %3d %2d \n", v + 1,
        m_varlist[v].s_bpos, m_varlist[v].s_npos,
        m_varlist[v].d_bpos, m_varlist[v].d_npos,
        m_varlist[v].VarIndex
      );
    }
  }
#endif _DEBUG
*/
    for (v = 0; v < m_nvarpos; v++) {
        if (m_varlist[v].d_npos == 0) continue;  // possible with HHIdent
    }

    v--;
    m_SafeRecordLength = m_varlist[v].d_bpos + m_varlist[v].d_npos;

#ifdef _DEBUG
// tijdelijke controle file
	fd = fopen("Controle.txt", "w");
	fprintf(fd, "%s\n", "controle file");
	fprintf(fd, "%s \n", "  bpos  npos  var  bpos  npos");
	fprintf(fd, "%5d\n", m_nvarpos);
	for (i=0; i< m_nvarpos; i++) {
		fprintf(fd,"%5d %5d %5d %5d %5d\n", m_varlist[i].s_bpos, m_varlist[i].s_npos,
			m_varlist[i].VarIndex, m_varlist[i].d_bpos, m_varlist[i].d_npos);
	}
	fprintf(fd, "%5d %5d \n", m_SafeRecordLength, v);
   fclose (fd);
#endif //_DEBUG
    return true;
}

bool CMuArgCtrl::MakeFreeRecordDescription(long HHIdentOption)
{
	long v;
	long np;
	m_nvarpos  = m_nvar;
	if (m_varlist != 0) {
		delete [] m_varlist;
	}
	m_varlist = new CVarList[m_nvarpos];
	if (m_varlist == 0) {
		return false;

	}

	CVariable *var;
	for (v = 0; v < m_nvarpos; v++) {
		var = &(m_var[v]);
		m_varlist[v].VarIndex = v;
		np = var->ComputeWidth(var->nPos, HHIdentOption,m_nRecFile);
		m_varlist[v].d_npos = np;
	}
	return true;

}

void CMuArgCtrl::QuickSortStringArray(std::vector<std::string> &s, int first, int last)
{ 
    int i, j;
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

bool CMuArgCtrl::DoEntropy(long VarNr, double& Entropy)
{
    int v = VarNr, i, j, n, N = 0, freq;
    CTable *t;

    if (v < 0 || v >= m_nvar || !m_var[v].IsCategorical) {
	return false;
    }

    Entropy = 0;

    for (i = 0; i < m_nUC; i++) {
        if (m_UCList[i].nDim != 1) continue;
        if (v != m_UCList[i].Varnr[0]) continue;

	// that's the table with freqs, compute entropy now
	if (m_tab[m_UCList[i].TabNr].nDim == m_UCList[i].nDim) {  // permanent table, can be 1-dimensional
            t = &(m_tab[m_UCList[i].TabNr]);
	} else {
            t = &m_UCList[i].table;
	}
	n = t->nCell - m_var[v].nMissing;  // Missing niet laten meedoen, zei AHNL

	// first the part with freq
	for (j = 0; j < n; j++) {
            freq = t->Cell[j];
            if (freq != 0) {  // avoid log(0)
                Entropy += freq * log(freq);
		N += freq;
            }
	}

	assert(N > 0);

	if (N < 1) return true; // all codes are on missing, apparently

	// then the part with N
	Entropy = log(N) / log(2) - Entropy / (N * log(2));
	break;
    }

    if (i == m_nUC) { // VarNr not found
	return false;
    }

    return true;
}

// fase = 0 first time
// fase = 1 second time (only for HH's)

bool CMuArgCtrl::MakeRecordSafe(char *record, int fase, int recnr, int nRecHH, long HHNum)
{
    int i, n;
    double score = 0, freqscore = 0, minscore = 0;

    // compute (recode)indices out of alfanumerical code for every categorical variable
    if (!ComputeVarIndices(record) ) return false;

    // compute unsafe combinations in m_UCList
    if (m_WithEntropy || m_WithPriority) {
        n = ComputeRecordUC(HHNum);
    }
    else{
        n = 0;
    }

    // set SetMissing on false and freq on zero for every variable
    for (i = 0; i < m_nvar; i++) {
        m_var[i].SetMissing = false;
	m_var[i].freq = 0;
    }

    // there are n unsafe combinations in this record
    if (n > 0) {
        if (fase == 0) { // in case of households fase can be > 0
            m_nUnsafe++;
	}

	// at first: a one dimensional table unsafe?
	for (i = 0; i < m_nUC; i++) {
            CUCList *m = &(m_UCList[i]);
            if (m->unsafe && m->nDim == 1) {
        // set this variable on missing and sets all tables with this variable at safe
                int iVar = m->Varnr[0];
		SetVarMissing(iVar); // print

		if (m_WithPriority) {
                    score += m_var[iVar].Priority;  // waarom???
		}
		if (m_WithEntropy) {
                    score += m_var[iVar].Entropy;  // waarom???
		}
            }
        } // end search one-dimensional unsafe tables

	// remember unsafe UCs
	//CUIntArray UC;
        std::vector<unsigned int> UC;
	for (i = 0; i < m_nUC; i++) {
            if (m_UCList[i].unsafe) {
                UC.push_back(i);
            }
	}

	if (UC.size() == 0) { // all tables safe? Ready!
            return true;
	}

	// remember SetMissings from one dimensional tables
	//CUIntArray SetMissings1Dim;
        std::vector<unsigned int> SetMissings1Dim;
	for (i = 0; i < m_nvar; i++) {
            if (m_var[i].SetMissing) {
                SetMissings1Dim.push_back(i);
            }
	}

	//CUIntArray FreqMissings;
        std::vector<unsigned int> FreqMissings;
	freqscore = SetFreqMissings(FreqMissings, nRecHH);    // function 1 to detect the vars to be set on Missing1

	// reset UCs set bij SetFreqMissings
	for (i = 0; i < UC.size(); i++) {
            m_UCList[UC[i]].unsafe = true;
	}

    	// reset SetMissing and freq
	for (i = 0; i < m_nvar; i++) {
            m_var[i].SetMissing = false;
            m_var[i].freq = 0;
	}
	// reset one dimensional vars
	for (i = 0; i < SetMissings1Dim.size(); i++) {
            SetVarMissing(SetMissings1Dim[i]);
	}

	//CUIntArray MinMissings;
        std::vector<unsigned int> MinMissings;
	minscore = SetMinMissings(MinMissings, nRecHH);  // function 2 to detect the vars to be set on Missing1

	// reset SetMissing and freq
	for (i = 0; i < m_nvar; i++) {
            m_var[i].SetMissing = false;
            m_var[i].freq = 0;
	}

	if (freqscore - minscore > 0.1) {  // minscore better
            for (i = 0; i < MinMissings.size();i++) {
                SetVarMissing(MinMissings[i]);//print + info welke
            }
            score += minscore;
	}
	else {
            for (i = 0; i < FreqMissings.size();i++) {
                SetVarMissing(FreqMissings[i]); //print + info welke
            }
            score += freqscore;
        }

        // reset SetMissings 1-dim           // AWTG 19-7-2001
	for (i = 0; i < SetMissings1Dim.size(); i++) {
            m_var[SetMissings1Dim[i] ].SetMissing = true;
        }

/*
#ifdef SHOWUNSAFE
    if (fabs(freqscore - minscore) >= 0.5) {
      // reset UCs
      for (i = 0; i < UC.GetSize(); i++) {
        m_UCList[UC[i]].unsafe = true;
      }
      ShowUCs(recnr);
      fprintf(fd_test, "Score: %.2f (freq=%.2f min=%.2f dif=%.2f)\n",
         score, freqscore, minscore, freqscore - minscore);
      for (i = 0; i < m_nvar; i++) {
        if (m_var[i].SetMissing) {
          fprintf(fd_test, "Op missing: %c\n", i +'A');
        }
      }
    }
#endif // SHOWUNSAFE
*/
    }
    return true;
}

// compute (recode)indices out of alfanumerical code for every categorical variable
 bool CMuArgCtrl::ComputeVarIndices(char *record)
{
    int v;

    for (v = 0; v < m_nvar; v++) {
        CVariable *var = &(m_var[v]);
	if (!var->IsCategorical) continue;
	ComputeTableIndex(record, var, v);
	assert(var->TableIndex >= 0);
	if (var->TableIndex < 0) {
            return false;   // program error
	}
    }

    return true;
}

// sets unsafe on true or false in m_UCList
int CMuArgCtrl::ComputeRecordUC(long HHNum)
{
    int i, j, n = 0, nDim, CellNr, HHSize;
    CTable t;   //Watch OUT
//	CTable  *t;
    CUCList *u;
    double TBIRThres, TBHRThres, logBHR;
    long BIRCounter = -1;
    for (i = 0; i < m_nUC; i++) {
	//CTable t;
	u = &(m_UCList[i]);
	if (!u->biggestThreshold) continue;  // table irrelevant, there's another with bigger threshold
	if (u->HasPram) continue;  // table irrelevant, contains prammed variables
	u->unsafe = false;
	nDim = u->nDim;
/*
		if (t.Cell != 0) {
        free(t.Cell);
      }
      if (t.BIRCell != 0) {
        free(t.BIRCell);
      }*/
        TBIRThres = m_tab[u->TabNr].BIRThreshold;
	TBHRThres = m_tab[u->TabNr].BHRThreshold ;

	if (nDim == m_tab[u->TabNr].nDim) {  // base table?
            t = m_tab[u->TabNr];
            if (t.IsBIR){
                BIRCounter ++;
            }
	}
	else {
            t = u->table;
	}

	// compute cell index
	CellNr = 0;
	int DimNr[MAXDIM];
	bool HasMissing = false;
	for (j = 0; j < nDim; j++) {
            CellNr *= t.SizeDim[j];
            CellNr += m_var[t.Varnr[j]].TableIndex;
            DimNr[j] = m_var[t.Varnr[j]].TableIndex;  // remember for BIR
            if (m_var[t.Varnr[j]].TableIsMissing) {
                HasMissing = true;                      // remember for BIR
            }
	}

	assert(CellNr >= 0 && CellNr < t.nCell);

	// Cell unsafe?
	if (t.IsBIR) {
            assert(t.GetCellNr(DimNr) == CellNr);  // DimNr correct?
            long freq = t.Cell[CellNr];
            double weight, v, logv;
            bool temp;
            if (freq > 0) {
                // returns false if all variables on Missing
		if (AddMissing(t, DimNr, freq, weight, HasMissing) ) { // all corresponding Missing(s) are added
                    temp = BaseIndividualRisk(freq, weight, &v);
                    assert(temp);
                    //Check the BIR first
                    if (v != 0) {
                        logv = log(v);
			if (logv >= TBIRThres){ //t.BIRThreshold) {
                            u->unsafe = true; // record unsafe for this combination, subtables too
                            n++;
			}
                        //And also for the BHR!
			if (!u->unsafe ){
                            if (m_lNumberOfHH > 0){
                                HHSize = m_HH[HHNum].m_lNumberofMembers;
				logBHR = 0;
				if (m_HH[HHNum].m_dBHR[0]> 0){
                                    logBHR = log(m_HH[HHNum].m_dBHR[0]);
				}
				logv = log(HHSize*v);
				if (logv >= TBHRThres && logBHR >= TBHRThres ) { //t.BHRThreshold) {
                                    u->unsafe = true; // record unsafe for this combination, subtables too
                                    n++;
				}
                            }

			}
                    }
		}
            }
            // For House holds
            // Ramya You have to change this
            // HIER moet het heel anders
            // Unsafe if Bir < hhTresh / n

//			if (m_lNumberOfHH > 0)	{
//				if (log((m_HH[HHNum].m_dBHR[BIRCounter])/m_HH[HHNum].m_lNumberofMembers) >= t.BHRThreshold)	{
//					u->unsafe = true;
//					n++;
//				}
//			}
        }
	else {
            if (t.Cell[CellNr] > 0 && t.Cell[CellNr] <= t.Threshold && !HasMissing) {
                //moeten we hier niet op missing letten? AHNL 2 dec 2004
		//!HasMissing toegevoegd 24 jan 2005
		u->unsafe = true;
		n++;
            }
	}
    } // m_nUC
  return n;
}

// sets flag to make variable missing
// sets all tables with this variable at safe
void CMuArgCtrl::SetVarMissing(int iVar)
{
    int i;

    assert(iVar >= 0 && iVar < m_nvar);
    assert(m_var[iVar].IsCategorical);

    m_var[iVar].SetMissing = true;
    if (m_var[iVar].RelatedTo >= 0) {
//		SetVarMissing(m_var[iVar].RelatedTo); Gevaarlijk
        i = m_var[iVar].RelatedTo;
	m_var[i].SetMissing = true;
    }

    int d;
    // all tables with this variable are safe now
    for (i = 0; i < m_nUC; i++){
        if (m_UCList[i].unsafe){
            for (d = 0; d < m_UCList[i].nDim; d++) {
                if (m_UCList[i].Varnr[d] == iVar) {
                    break;
		}
            }
            if (d < m_UCList[i].nDim) {  // variable is in the table
                m_UCList[i].unsafe = false;
            }
	}
    }
}

// sets missings
double CMuArgCtrl::SetFreqMissings(std::vector<unsigned int> &FreqMissing, int nRecHH)
{
    int i, d, nUnsafeTab, ni = -1;
    double score = 0.0;

    while (1) {  // until all tabs are safe: nUnsafeTab == 0
        nUnsafeTab = 0;

	// zero freq
	for (i = 0; i < m_nvar; i++) {
            m_var[i].freq = 0;
	}
	for (i = 0; i < m_nUC; i++) {
            if (m_UCList[i].unsafe) {
                nUnsafeTab++;
		// count freq variable
		for (d = 0; d < m_UCList[i].nDim; d++) {
                    m_var[m_UCList[i].Varnr[d] ].freq++;
		}
            }
	}

	if (nUnsafeTab == 0) {
            break; // ready!
	}

	// which variables on top?
	int maxfreq = -1;
	int maxprio = -1;
	double maxentrop = -1;

	// meest voorkomende variabele wordt op missing gezet
	// bij meerdere kandidaten die met kleinste prio/entro
	for (i = 0; i < m_nvar; i++) {
            if (!m_var[i].IsCategorical) continue;
            if (m_var[i].freq > maxfreq) {
                maxfreq = m_var[i].freq;
		maxprio = m_var[i].Priority;
		maxentrop = m_var[i].Entropy;
		ni = i;
            }
            else {
                if (m_var[i].freq == maxfreq) {
                    if (m_WithPriority) {
                        if (m_var[i].Priority * GetHHSizeFactor(i, nRecHH) < maxprio) {
                            maxprio = m_var[i].Priority * GetHHSizeFactor(i, nRecHH);
                            ni = i;
			}
                    }
                    if (m_WithEntropy) {
                        if (m_var[i].Entropy  * GetHHSizeFactor(i, nRecHH) < maxentrop) {
                            maxentrop = m_var[i].Entropy * GetHHSizeFactor(i, nRecHH);
                            ni = i;
			}
                    }
		}
            }
	}
	assert(ni >= 0 && ni < m_nvar);
	SetVarMissing(ni);
	FreqMissing.push_back(ni);
	if (m_WithPriority) {
            score += m_var[ni].Priority * GetHHSizeFactor(ni, nRecHH);
	}
	if (m_WithEntropy) {
            score += m_var[ni].Entropy * GetHHSizeFactor(ni, nRecHH);
	}
    }  // end while

    return score;
}

// sets missings, second method
double CMuArgCtrl::SetMinMissings(std::vector<unsigned int> &MinMissing, int nRecHH)
{
    int i, j, nUnsafeTab, ni, ndim, iVar;
    double score = 0.0;
    // at least one tabel is unsafe!

    // for number of dimensions 2, 3, ...
    for (ndim = 2; ndim < MAXDIM; ndim++) {
        nUnsafeTab = 0;
	for (i = 0; i < m_nUC; i++) {
            CUCList *m = &(m_UCList[i]);
            if (!m->unsafe) continue;
            nUnsafeTab++;
            if (m->nDim == ndim) {
            // bereken gunstigste var
                double max = 100000;
		ni = 0;
		for (j = 0; j < m->nDim; j++) {
                    iVar = m->Varnr[j];
                    if (m_WithPriority) {
                        if (m_var[iVar].Priority * GetHHSizeFactor(iVar, nRecHH) < max) {  // rekening houden nREcHH??
                            max = m_var[iVar].Priority  * GetHHSizeFactor(iVar, nRecHH);
                            ni = iVar;
			}
                    }
                    if (m_WithEntropy) {
                        if (m_var[iVar].Entropy * GetHHSizeFactor(iVar, nRecHH) < max) {  // rekening houden nREcHH??
                            max = m_var[iVar].Entropy * GetHHSizeFactor(iVar, nRecHH);
                            ni = iVar;
			}
                    }
		}
		// set this variable on missing and
		// sets all tables with this variable at safe

		SetVarMissing(ni);
		MinMissing.push_back(ni);
		if (m_WithPriority) {
                    score += m_var[ni].Priority * GetHHSizeFactor(ni, nRecHH); // !! x huishoudgrootte
		}
		if (m_WithEntropy) {
                    score += m_var[ni].Entropy * GetHHSizeFactor(ni, nRecHH);  // !! x huishoudgrootte
		}
            }
	}
	if (nUnsafeTab == 0) break;
    }
    assert(ndim < MAXDIM);
    return score;
}

int CMuArgCtrl::GetHHSizeFactor(int VarIndex, int nRecHH)
{
    if (m_HHIdentOption == HHIDENT_NO) return 1;
        return m_var[VarIndex].IsHHVar ? nRecHH : 1;
}

bool CMuArgCtrl::WriteRecord(FILE *fd_out, char *record, long HHIdentOption, long recnr, bool WithBHR, long HHNum, bool PrintBIR, char *origrecord)
{
    char str[MAXRECORDLENGTH];
    int i, iVar;
    CVarList *v = &(m_varlist[0]);// to suppress warning
    CChSafeVarInfo *objVarInfo;
    //CString stempstr,mainstr, varstr;
    std::string stempstr,mainstr, varstr;
    CVariable *tempvar;
    char connumstr[MAXCODEWIDTH];
    //CString InString,OutString,TempString,tempcode;
    std::string InString,OutString,TempString,tempcode;
    InString = record;
    OutString = "";

    //long lrecordlength = m_varlist[m_nvarpos].d_bpos + m_varlist[m_nvarpos].d_npos +2;

    long lfilenum,lArrIndex;

    for (i=0; i<m_nChangeFiles; i++) {
        objVarInfo = &(m_ChangeFiles[i]);
	if (!objVarInfo->FillVariableCode()) {
            return false;
	}
    }
    // Here you have to see if the variable is in one of the objects and then replace it.
    for (i = 0; i < m_nvarpos; i++) {
        v = &(m_varlist[i]);
	if (IsInOutputFile(v->VarIndex,&lfilenum,&lArrIndex)) {
            objVarInfo = &(m_ChangeFiles[lfilenum]);
            // set curr file pointer
            //objVarInfo->SetCurrFilePos(lrecordlength*(recnr-1));
            // Probably not yet filled files
            // if (objVarInfo->FillVariableCode()) {
            stempstr = objVarInfo->sVariableCode.at(lArrIndex);
            //stempstr = stempstr.Left(v->d_npos);
            // Here things have to change
            
            // Check if it is a numeric variable

            tempvar = &(m_var[v->VarIndex]);
            if (tempvar->IsNumeric) {// bla bla
                strcpy(connumstr,stempstr.c_str());
		double dub = atof(connumstr);
		if (tempvar->HasRound) {
                    //dub = DoRound(dub, tempvar->RoundBase);
                    sprintf(connumstr, "%*.*f", v->d_npos, tempvar->RoundnDec, dub);
		}
		else {
                    sprintf(connumstr, "%*.*f", v->d_npos, tempvar->nDec, dub);
		}
		AddSpacesBefore(connumstr, v->d_npos);
		if (m_OutFileIsFixedFormat) {
                    memcpy(&str[v->d_bpos], connumstr, v->d_npos);
		}
		else {
                    TempString = connumstr;
                    OutString = OutString + m_OutFileSeperator +TempString;
		}

		//stempstr.Format("%*d", v->d_npos, m_HHSeqNr);
		//memcpy(&str[v->d_bpos], (LPCTSTR)stempstr, v->d_npos);
            }
            else{
                //if (stempstr.GetLength() > v->d_npos) {
                if (stempstr.size() > v->d_npos) {
                    //stempstr = stempstr.Left(v->d_npos);
                    stempstr = stempstr.substr(0,v->d_npos);
                    if (m_OutFileIsFixedFormat) {
                        memcpy(&str[v->d_bpos], stempstr.c_str(), v->d_npos);
                    }
                    else {
                        if (m_StringsInQuotes) {
                            stempstr = '"'+stempstr + '"';
                        }
                        OutString = OutString + m_OutFileSeperator + stempstr;
                    }
                }
                else {
                    strcpy(connumstr,stempstr.c_str());
                    AddSpacesBefore(connumstr, v->d_npos);
                    if (m_OutFileIsFixedFormat) {
                        memcpy(&str[v->d_bpos], connumstr, v->d_npos);
                    }
                    else {
                        TempString = connumstr;
			if (m_StringsInQuotes) {
                            TempString = '"' + TempString + '"';
			}
			OutString = OutString + m_OutFileSeperator +TempString;
                    }
		}
            }
	}
	//else
	else {
            assert(v->d_npos >= 0);
            if(v->d_npos == 0) continue;
            
            iVar = v->VarIndex;
            // unused positions
            if (iVar < 0) {
                if (m_OutFileIsFixedFormat) {
                    memcpy(&str[v->d_bpos], &record[v->s_bpos], v->d_npos);
                    continue;
		}
		else {
                    return false;
		}
            }

            CVariable *var = &m_var[iVar];

            // HHIdent
            if (iVar == m_HHIdentVar) {
                if (HHIdentOption == HHIDENT_CHANGESEQNO) {
                    std::ostringstream ss;
                    std::string s;
                    //s.Format("%*d", v->d_npos, m_HHSeqNr);
                    ss.width(v->d_npos);
                    ss << m_HHSeqNr;
                    s = ss.str();
                    
                    if (m_OutFileIsFixedFormat) {
                        strncpy(&str[v->d_bpos], s.c_str(), v->d_npos);
                    }
                    else {
                        if ((m_StringsInQuotes)&& (!var->IsNumeric)) {
                            s = '"'+ s +'"';
                        }
                        OutString = OutString + m_OutFileSeperator + s;
                    }

		continue;    // in all other cases already oke
                }
            }

    // Post RAndomization Method, do first
            if (var->HasPram) {
                int iCode = -1, bw = var->PramBandWidth;
                int n = m_var[iVar].GetnCodes(false);
                if (var->TableIndex < n) {  // not missing value
                    if (rand() % 100 >= (int) var->PramValue[var->TableIndex]) { // change code in another
                        if (bw == -1) { // no BandWidth
                            n--;  // for the code itself
                            iCode = GetRandomInteger() % n;  // n can be > 32767, result 0 .. n - 1
                            if (iCode >= var->TableIndex) {
                                iCode++;
                            }
                        } else {   // BandWidth specified
                            int w;
                            do {
                                w = GetRandomInteger() % (bw << 1); // change to left and right
                                if (w >= bw) w++; // skip the code itself
                                iCode = var->TableIndex - bw + w;
                            } while (iCode < 0 || iCode >= n);
                        }
                    } else {
                        iCode = var->TableIndex;
                    }

                    if (var->HasRecode) {
                        assert(iCode >= 0 && iCode < var->Recode.nCode - var->Recode.nMissing);
                        if (m_OutFileIsFixedFormat) {
                            strncpy(&str[v->d_bpos], var->Recode.sCode[iCode].c_str(), v->d_npos);
                        }
                        else {
                            TempString = var->Recode.sCode[iCode];
                            if ((m_StringsInQuotes)&& (!var->IsNumeric)) {
                                TempString = '"' + TempString + '"';
                            }
                            OutString = OutString + m_OutFileSeperator + TempString;
                        }
                    } else {
                        assert(iCode >= 0 && iCode < var->nCode - var->nMissing);
                        if (m_OutFileIsFixedFormat) {
                            strncpy(&str[v->d_bpos], var->sCode[iCode].c_str(), v->d_npos);
                        }
                        else {
                            TempString = var->sCode[iCode];
                            if ((m_StringsInQuotes)&& (!var->IsNumeric)) {
                                TempString = '"' + TempString + '"';
                            }
                            OutString = OutString + m_OutFileSeperator + TempString;
                        }
                    }
                    continue;
                } // missing
            }  // HasPram

            // a "normal" variable
            if (!var->HasRecode) {
            // no recode variable
                if (var->SetMissing) { // variable has to be set on missing
                    if (m_OutFileIsFixedFormat) {
                        memcpy(&str[v->d_bpos], var->Missing1.c_str(), v->d_npos);
                    }
                    else {
                        TempString = var->Missing1;
                        if ((m_StringsInQuotes)&& (!var->IsNumeric)) {
                            TempString = '"' + TempString + '"';
                        }
                        OutString = OutString + m_OutFileSeperator + TempString;
                    }
                    var->nSuppress++;
                } 
                else {
                    if (var->IsNumeric) {
                        // Rekening houden met missings, die botweg kopi�ren
                        char code[MAXCODEWIDTH];
                        if (m_OutFileIsFixedFormat){
                            strncpy(code, (char *) (&record[v->s_bpos]), v->s_npos);
                            code[v->s_npos] = 0;
                        }
                        else {
                            if (ReadVariableFreeFormat(record,i,&(tempcode))) {
                                strcpy(code,tempcode.c_str());
                                code[var->nPos] = 0;
                            }
                        }
                        if (strcmp(code, var->Missing1.c_str()) == 0 || strcmp(code, var->Missing2.c_str()) == 0) {
                            AddSpacesBefore(code, v->d_npos);
                            if (m_OutFileIsFixedFormat)	{
                                memcpy(&str[v->d_bpos], code, v->d_npos);
                            }
                            else {
                                TempString = code;
                                OutString = OutString + m_OutFileSeperator + TempString;
                            }
                        }
                        else { // not Missings:
                            double d = atof(code);
                            // Rounding?
                            if (var->HasRound) {
                                d = var->DoRound(d);
                                sprintf(code, "%*.*f", v->d_npos, var->RoundnDec, d);
                            } else {
                                sprintf(code, "%*.*f", v->d_npos, var->nDec, d);
                            }

                            // TopCoding?
                            if (var->HasCodingTop && d >= var->TopLevel) {
                                strcpy(code, var->TopString.c_str());
                            }
                            // BottomCoding
                            if (var->HasCodingBottom && d <= var->BottomLevel) {
                                strcpy(code, var->BottomString.c_str());
                            }

                            // WeightNoise
                            if (var->HasWeightNoise) {
                                //d = DoWeightNoise(d, var->WeightNoise);
                                d = var->DoWeightNoise(d);
                                sprintf(code, "%*.*f", v->d_npos, var->nDec, d);
                            }
                            // put code in output record
                            assert(strlen(code) <= (unsigned) v->d_npos);
                            AddSpacesBefore(code, v->d_npos);
                            if (m_OutFileIsFixedFormat) {
                                memcpy(&str[v->d_bpos], code, v->d_npos);
                            }
                            else {
                                TempString = code;
                                OutString = OutString + m_OutFileSeperator + TempString;
                            }
                        }
                    }
                    else {  // not specified as numeric
                        if (m_OutFileIsFixedFormat) {
                            memcpy(&str[v->d_bpos], &record[v->s_bpos], v->d_npos);
                        }
                        else {
                            if (ReadVariableFreeFormat(record,i,&(tempcode))) {
                                if ((m_StringsInQuotes)&& (!var->IsNumeric)) {
                                    tempcode = '"' + tempcode + '"';
                                }
                                OutString = OutString + m_OutFileSeperator + tempcode;
                            }
                        }
                    }
                }
            } else {
                // recode variable
                if (var->SetMissing) {
                    if (m_OutFileIsFixedFormat) {
                        memcpy(&str[v->d_bpos], var->Recode.Missing1.c_str(), v->d_npos);
                    }
                    else {
                        TempString = var->Recode.Missing1;
                        if ((m_StringsInQuotes)&& (!var->IsNumeric)) {
                            TempString = '"' + TempString + '"';
                        }
                        OutString = OutString + m_OutFileSeperator + TempString;
                    }
                    var->nSuppress++;
                }
                else {
                    int t = var->TableIndex;
                    assert(t >= 0 && t < var->Recode.nCode);
                    if (m_OutFileIsFixedFormat) {
                        memcpy(&str[v->d_bpos], var->Recode.sCode[t].c_str(), v->d_npos);
                    }
                    else {
                        TempString = var->Recode.sCode[t];
                        if ((m_StringsInQuotes)&& (!var->IsNumeric)) {
                            TempString = '"' + TempString + '"';
                        }
                        OutString = OutString + m_OutFileSeperator + TempString;
                    }
                }
            }
        }  // m_nvarpos
	if (m_OutFileIsFixedFormat) {
            str[v->d_bpos + v->d_npos] = 0;
	}
	else {
            //OutString.TrimRight();
            OutString = trimright(OutString);
	}

        //assert(v->d_bpos + v->d_npos == m_SafeRecordLength);
    }
    // for test

    // Make boolean and add to function as a variable
    //#ifdef TESTBIR
    // add freq, weight and BIR:
    if (PrintBIR)	{
        int i, BIRFreq;
	double BIRWeight, BIR;
	for (i = 0; i < m_ntab; i++) {
            if (m_tab[i].IsBIR) {
                break;
            }
	}

	if (i == m_ntab) {  // no BIR specified
            // fprintf(fd_out,"%s\n", str);
            return false;
	}

	if (m_tab[i].HasRecode) {
            i += m_ntab;
	}

	if (m_InFileIsFixedFormat) {
            fprintf(fd_out,"%s", str);
	}
	else {
            // Remove First element
            //fprintf(fd_out,"%s\n",OutString);
            //fprintf(fd_out,"%s", OutString.Right(OutString.GetLength() - 1));
            fprintf(fd_out,"%s", OutString.substr(1,OutString.size() - 1).c_str());
	}

	if (m_lNumberOfHH > 0)  // seems to be a problem with the strange way of writing HH
            // have to compute var indices again
	{
            if (!ComputeVarIndices(origrecord) ) return false;
	}
	ComputeTableBIR(m_tab[i], BIRFreq, BIRWeight, BIR); // + HH BIR
//	fprintf(fd_out, ", f = %4d, F = %12.6f, BIR = %14.12f", BIRFreq, BIRWeight, BIR);
	fprintf(fd_out, " %4d %14.4f %14.12f", BIRFreq, BIRWeight, BIR);
	if (WithBHR)	{
            for (i=0; i<m_lNumBIRs; i++){
                if (HHNum < m_lNumberOfHH){
//                  fprintf(fd_out,",BHR = %14.12f",m_HH[HHNum].m_dBHR[i]);
                    fprintf(fd_out," %14.12f",m_HH[HHNum].m_dBHR[i]);
//                  fprintf(fd_out," %14.12f",m_HH[HHNum].m_dBHR[i]/m_HH[HHNum].m_lNumberofMembers );
		}
            }
	}
	else {
            fprintf(fd_out," %14s", "              ");
	}

	fprintf(fd_out, "\n");

    } // end else
//#else  // TESTBIR
    else{
        if (m_WriteRandom) {
            fseek(fd_out, (m_SafeRecordLength + 2) * m_WriteRecNr, SEEK_SET);
            m_WriteRecNr += m_WriteCoPrime;
            m_WriteRecNr %= m_nRecFile;
            str[m_SafeRecordLength] = '\r';
            str[m_SafeRecordLength + 1] = '\n';
            str[m_SafeRecordLength + 2] = '\0';
            fwrite(str, m_SafeRecordLength + 2, 1, fd_out);
	}
	else {
            if (m_InFileIsFixedFormat) {
                fprintf(fd_out,"%s\n", str);
            }
            else {
                // Remove First element
		//fprintf(fd_out,"%s\n",OutString);
		//fprintf(fd_out,"%s\n", OutString.Right(OutString.GetLength() - 1));
                fprintf(fd_out,"%s\n", OutString.substr(1,OutString.size() - 1).c_str());
            }
	}
    }
//#endif // TESTBIR

  return true;
}


bool CMuArgCtrl::IsInOutputFile(long VarIndex, long *FileNum, long *ArrIndex)
{
    long i,j;
    bool IsinFiles = false;
    CChSafeVarInfo *objVarInfo;
    for (i=0; i<m_nChangeFiles; i++) {
	objVarInfo = &(m_ChangeFiles[i]);
	for (j=0; j< objVarInfo->GetNumberVar(); j++) {
            if (VarIndex == objVarInfo->m_lVarIndex[j]) {
                *ArrIndex = j;
		IsinFiles = true;
		break;
            }
	}
	if (IsinFiles) {
            *FileNum = i;
            break;
	}
    }
    return IsinFiles;
}

// value 0 ... 2^31 - 1
int CMuArgCtrl::GetRandomInteger()
{ // rand() returns value 0 - 32767
    return ((rand() % 16384) << 16) + rand();
    // returns 0 - 2147483647
}

int CMuArgCtrl::ComputeRecHH(FILE *fd, int bpos, int npos)
{
    char str[MAXRECORDLENGTH];
    char FirstHH[MAXCODEWIDTH];      // first household ident
    int res, nrec;

    res = ReadMicroRecord(fd, str); // first record of household
    switch (res) {
	case INFILE_ERROR:
	case INFILE_EOF:
            return 0;
    }

    strncpy(FirstHH, (char *) &str[bpos], npos);
    nrec = 1;

    while (res = ReadMicroRecord(fd, str), res == INFILE_OKE) {
        if (strncmp(FirstHH, (char *) &str[bpos], npos) != 0) break;
        nrec++;
    }

    return nrec;
}

// Look at this part.
// I think this is where the household is made safe.
bool CMuArgCtrl::DoCompleteHH(FILE *fd, FILE *fd_out, long StartPos, int n_rec,	int *InvolvedVar, int recnr, long HHIdentOption, long HHNum, bool PrintBIR)
{
    int i, j, ReadCode;
    //CString cstr;
    char str[MAXRECORDLENGTH];
    bool WithBHR = false;
    char orgstr[MAXRECORDLENGTH];

    /*
#ifdef _DEBUGG
  #define SHOWHH
#endif  // _DEBUG

#ifdef SHOWHH
  char code[MAXCODEWIDTH];
  int n = 0, testvar[10];
#endif // SHOWHH
*/

    fseek(fd, StartPos, SEEK_SET);
    
    for (i = 0; i < n_rec; i++) {
        if (i == 0){
            WithBHR = true;
	}
	else{
            WithBHR = false;
	}
	if (ReadCode = ReadMicroRecord(fd, str), ReadCode != INFILE_OKE) {
            return false;
	}
	strcpy((char *)orgstr,(char *)str);
	MakeRecordSafe(str, 1, recnr, n_rec, HHNum); // !! parameter n_rec = huishoudgrootte
	for (j = 0; j < m_HHVars.size(); j++) {
            if (InvolvedVar[j]) {
                m_var[m_HHVars[j]].SetMissing = true;
		/*
#ifdef SHOWHH
        if (i == 0) {
          testvar[n++] = m_HHVars[j] + 1;
        }
#endif // SHOWHH*/
            }
	}
	WriteRecord(fd_out, str, HHIdentOption, i, WithBHR, HHNum, PrintBIR, orgstr);
    }
/*
#ifdef SHOWHH
  strncpy(code, (char *) (&str[m_var[m_HHIdentVar].bPos]), m_var[m_HHIdentVar].nPos);
  code[m_var[m_HHIdentVar].nPos] = 0;
  TRACE("Read %d records with HHCode = %s\n", n_rec, code);
  for (i = 0; i < n; i++) {
    char s[10];
    sprintf(s, "%d ", testvar[i]);
    cstr += s;
  }
  TRACE("Vars %s\n", (LPCTSTR) cstr);
#endif // SHOWHH
*/
    return true;
}


bool CMuArgCtrl::NumberOfHH(char *str, long &HHNumbers)
{
    int i, bp, ap;
    char code[MAXCODEWIDTH];
    CVariable *var;
    std::string tempcode;
    std::string HHcode;
    bool IsFirstHH = false;
    for (i= 0; i<m_nvar; i++) {
	var = &(m_var[i]);
	if (var->IsHHIdent) {
            if(m_InFileIsFixedFormat) {
                bp = var->bPos;         // startposition
		ap = var->nPos;         // number of positions
		strncpy(code, (const char *)&str[bp], ap); // get code from record
		code[ap] = 0;
		//HHcode = CString(code);
                HHcode = code;
            }
            else {
                ap = var->nPos;         // number of positions
		if (ReadVariableFreeFormat(str,i,&(tempcode))) {
                    /*strcpy(code,(const char*)tempcode);
                    code[ap] = 0;*/
                    HHcode=tempcode;
		}
            }
            CurrentHHName = HHcode;
            if (LastHHName == "") {
                IsFirstHH = true;
            }
            if (IsFirstHH){
                LastHHName = CurrentHHName;
            }
            if (LastHHName != CurrentHHName) {
                HHNumbers = HHNumbers+1;
		LastHHName = CurrentHHName;
            }
	}
	else {
            continue;
	}
    }

    return true;
}


bool CMuArgCtrl::IsNewHH(char *str)
{
    int i, bp, ap;
    char code[MAXCODEWIDTH];
    CVariable *var;
    std::string tempcode;
    std::string HHcode;
    bool IsFirstHH = false;
    for (i= 0; i<m_nvar; i++) {
	var = &(m_var[i]);
	if (var->IsHHIdent) {
            if(m_InFileIsFixedFormat) {
		bp = var->bPos;         // startposition
		ap = var->nPos;         // number of positions
		strncpy(code, (const char *)&str[bp], ap); // get code from record
		code[ap] = 0;
		HHcode = code;
            }
            else {
		ap = var->nPos;         // number of positions
		if (ReadVariableFreeFormat(str,i,&(tempcode))) {
					/*strcpy(code,(const char*)tempcode);
					code[ap] = 0;*/
                    HHcode=tempcode;
		}
            }
	}
	else {
            continue;
	}
    }

    CurrentHHName = HHcode;
    if (LastHHName == "") {
        LastHHName = CurrentHHName;
	return false;
    }
    if (LastHHName == CurrentHHName) {
        return false;
    }
    else {
        LastHHName = CurrentHHName;
	return true;
    }
}

bool CMuArgCtrl::CalculateBaseHouseholdRisk(long *ErrorCode)
{
    FILE *fd_in;
    long i,j;
    char str[MAXRECORDLENGTH];
    double *tempBIRarray;
    tempBIRarray = new double [m_lNumBIRs];
    if (m_lNumberOfHH == 0){
        *ErrorCode = NOHOUSEHOLDS;
	return false;
    }

    if (m_lNumBIRs == 0){
	*ErrorCode = NOBIRTAB;
	return false;
    }

    if (m_fname[0] == 0){
	*ErrorCode = NODATAFILE;
	return false;
    }

    fd_in = fopen(m_fname, "r");
    if (fd_in == 0) {
	return false;
    }

    long recnr = 0;
    long res;
    rewind(fd_in);

    if ((!m_InFileIsFixedFormat) &&(m_IgnoreFirstLine )){
	res = ReadMicroRecord(fd_in, str);
    } // AHNL 30 maart 2005 Eerste record weglezen bij SAS file

    for (i=0; i<m_lNumberOfHH; i++) {
	// First allocate space for BIRs in HH
	m_HH[i].PrepareHouseholdBIR(m_lNumBIRs);
	for (j=0; j<m_HH[i].m_lNumberofMembers; j++) {
            res = ReadMicroRecord(fd_in, str);
            if (++recnr % FIREPROGRESS == 0) {
		FireUpdateProgress((int)(ftell(fd_in) *100.0/m_fSize));
            }
            switch(res) {
		case INFILE_ERROR:
                    // goto error;
                    delete[]tempBIRarray;
                    fclose(fd_in);
                    return false;
                    break;
                case INFILE_EOF: // should not happen
                    delete[]tempBIRarray;
                    fclose(fd_in);
                    break;
		case INFILE_OKE:
//                  if ((!m_InFileIsFixedFormat) &&(m_IgnoreFirstLine ) &&(recnr == 1))	{
//                      continue;
//                  }
//                  else {
                    if (!FindBIRForRec(str,tempBIRarray)) {
			return false;
                    }
                    m_HH[i].SaveBIR(tempBIRarray,m_lNumBIRs,j);
//				}

            } // end case
        } // end for j

	// maybe check for infile file endings before it should
	if (j < m_HH[i].m_lNumberofMembers-1){
            delete[] tempBIRarray;
            fclose(fd_in);
            return false;
	}
	if (!m_HH[i].CalculateBHR(m_lNumBIRs)){

	}
    }// end for i

    delete[] tempBIRarray;
    fclose(fd_in);
    return true;
}

bool CMuArgCtrl::FindBIRForRec(char *record, double *BIRarray)
{
    long i,j, freq, bircounter = 0;;
    CTable t;
    double weight,v;
    long CellNr;
    bool temp;
    int DimNr[MAXDIM];
    if (!ComputeVarIndices(record) ) return false;
    for (i=0; i<m_ntab; i++) {
	t = m_tab[i];
	if (t.HasRecode){
            t = m_tab[i+m_ntab];
	}
	if (t.IsBIR){
            CellNr = 0;
            bool  HasMissing = false;
            for (j =0; j<t.nDim; j++){
		CellNr *= t.SizeDim[j];
		CellNr += m_var[t.Varnr[j]].TableIndex;
		DimNr[j] = m_var[t.Varnr[j]].TableIndex;

		if (m_var[t.Varnr[j]].TableIsMissing) {
                    HasMissing = true;
		}
            }

            assert(t.GetCellNr(DimNr) == CellNr);
            freq = t.Cell[CellNr];
            if (freq >0 ) {
		if (AddMissing(t,DimNr,freq,weight, HasMissing)) {
                    temp = BaseIndividualRisk(freq,weight,&v);
                    assert(temp);
                    // Not sure if v has to be added to the array or log v
                    BIRarray[bircounter] = v; // log v
                    bircounter++;
		}
            }
            else {
                BIRarray[bircounter] = 0;
		bircounter++;
            }
	}
    }

    return true;
}


void CMuArgCtrl::ComputeTableBIR(CTable &t, int& BIRFreq, double& BIRWeight, double& BIR)
{ 
    int j, DimNr[MAXDIM];
    bool HasMissing = false;

    // compute dimension numbers, look for missings
    for (j = 0; j < t.nDim; j++) {
	DimNr[j] = m_var[t.Varnr[j]].TableIndex;
	if (m_var[t.Varnr[j]].TableIsMissing) {
            HasMissing = true;
	}
    }

    long freq = t.Cell[t.GetCellNr(DimNr)];
    double weight, v;
    bool temp;
    if (freq > 0) {
	// returns false if all variables on Missing
	if (AddMissing(t, DimNr, freq, weight, HasMissing) ) { // all corresponding Missing(s) are added
            temp = BaseIndividualRisk(freq, weight, &v);
            assert(temp);
            BIRFreq = freq;
            BIRWeight = weight;
            BIR = v;
	} else {  // something wrong
            BIRFreq = -1;
            BIRWeight = -1;
            BIR = -1;
	}
    }
}


bool CMuArgCtrl::GetBHRHistogramData(long TableIndex, long nClasses, double *ClassLeftValue, long *HHFrequency, long *RecFrequency)
{
    long i, k,index, BIRCounter=0;
    double *temparray;
    long ci;

    index = TableIndex -1;

    if (nClasses < 2){
	return false;
    }

    if ((index <0) || (index >= m_ntab)){
	return false;
    }

    if (!m_tab[index].IsBIR){
	return false;
    }

    // to find which element of HH.BHR to get
    for (i=0; i<m_ntab; i++){
        if (i== index){
            break;
	}
	if (m_tab[i].IsBIR){
            BIRCounter++;
	}
    }

    temparray = new double [m_lNumberOfHH];
    for (i=0; i<m_lNumberOfHH; i++){
	temparray[i] =  m_HH[i].m_dBHR[BIRCounter];
    }

    QuickSortDoubleArray(temparray,0,m_lNumberOfHH-1);

    for (i=0; i<m_lNumberOfHH; i++){
    	temparray[i] =  log(temparray[i]);
    }

    CTable *t = &(m_tab[index]);
    //ANCO:  Neem indien nodig  de recoded tabel!!!!
    if (t->HasRecode){
	t = &(m_tab[m_ntab + index]); // take the recoded one
    }

    t->BHRMaxValue = temparray[m_lNumberOfHH-1];
    t->BHRMinValue = temparray[0];
    t->BHRnClasses = nClasses;
    // compute class width
    t->BHRClassWidth= (t->BHRMaxValue - t->BHRMinValue) / nClasses;

    // compute classboundaries
    for (k = 0; k <= nClasses; k++){
	ClassLeftValue[k] = t->BHRMinValue + k * t->BHRClassWidth;
    }

    for (k=0; k< nClasses; k++){
	HHFrequency[k] = 0;
	RecFrequency[k] = 0;
    }

/*	for (k=0;  k< nClasses; k++)	{
		for (i=0; i<m_lNumberOfHH; i++)	{
		//	if ((log(m_HH[i].m_dBHR[BIRCounter]) >= ClassLeftValue[k])
					//&& (log(m_HH[i].m_dBHR[BIRCounter]) < ClassLeftValue[k+1]))
			if ((log(m_HH[i].m_dBHR[BIRCounter]) >= ClassLeftValue[k])
					&& (log(m_HH[i].m_dBHR[BIRCounter]) < ClassLeftValue[k+1]))
			{
				HHFrequency[k]= HHFrequency[k]+1;
				RecFrequency[k] = RecFrequency[k] + m_HH[i].m_lNumberofMembers;
			}
		}
	}*/

    for (i=0; i<m_lNumberOfHH; i++)	{
	ci = (int) ((log(m_HH[i].m_dBHR[BIRCounter]) - t->BHRMinValue) / t->BHRClassWidth);
	if (ci == t->BHRnClasses) ci--;  // can happen in rare cases, due to rounding
        //assert(ci >= 0 && ci < t.BIRnClasses);
        HHFrequency[ci] = HHFrequency[ci] +1;
        RecFrequency[ci] = RecFrequency[ci] + m_HH[i].m_lNumberofMembers;
    }

    delete [] temparray;
    return true;
}


void CMuArgCtrl::QuickSortDoubleArray(double * d, int first, int last)
{ 
    int i, j;
    double mid, temp;
    assert(first >= 0 && last >= first);

    do{
        i = first;
        j = last;
        mid = (d[(i + j) / 2]);
        do {
            while (d[i] < mid) i++;
            while (d[j] > mid) j--;
            if (i < j) {
                temp = d[i];
                d[i] = d[j];
                d[j] = temp;
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
                QuickSortDoubleArray(d, first, j);
            }
            first = i;
        } else {
            if (i < last) {
                QuickSortDoubleArray(d, i, last);
            }
            last = j;
        }
    } while (first < last);

}


bool CMuArgCtrl::SetBHRThreshold(long TableIndex, double BHRThreshold, long *nUnsafeHH, long *nUnsafeRec)
{
    long index = TableIndex -1;
    long i;
    long BIRCounter =0;
    long tempRec, tempHH;
    if ((index <0) || (index >= m_ntab))	{
	return false;
    }

    if (!m_tab[index].IsBIR)	{
	return false;
    }

    // to find which element of HH.BHR to get
    for (i=0; i<m_ntab; i++)	{
	if (i== index) {
            break;
	}
	if (m_tab[i].IsBIR)	{
            BIRCounter++;
	}
    }

	tempRec =0;
	tempHH = 0;
	CTable *t = &(m_tab[index]);
	if (t->HasRecode) {
		t = &(m_tab[m_ntab + index]); // take the recoded one
	}
	t->BHRThreshold = BHRThreshold;

	for (i=0; i<m_lNumberOfHH; i++) {
		if (log(m_HH[i].m_dBHR[BIRCounter])  >= BHRThreshold)	{
			tempHH++;
			tempRec = tempRec + m_HH[i].m_lNumberofMembers;
		}
	}

	*nUnsafeHH = tempHH;
	*nUnsafeRec = tempRec;
	return true;
}

bool CMuArgCtrl::CalculateBIRFreq(long TableIndex, long nUnsafe, double *BIRResult, long *ErrorCode)
{
    long i,ind = TableIndex-1;
    CTable *tab;
    FILE *fdread;

    double *tempBIRarray;
    long  *tempFreqarray;
    char str[MAXRECORDLENGTH];
    double BIRres;

    if (ind < 0 || ind >= m_ntab){
        *ErrorCode = NOTABLES;
	return false;
    }
    tab = &(m_tab[ind]);
    if (tab->HasRecode){
	tab = &(m_tab[m_ntab + ind]); // take the recoded one
    }

    if (!tab->IsBIR){
	*ErrorCode = NOBIRTAB;
	return false;
    }

    if (m_fname[0] == 0){
	*ErrorCode = NODATAFILE;
        return false;
    }

    fdread = fopen(m_fname, "r");
    if (fdread == 0){
	*ErrorCode = FILENOTFOUND;
        return false;
    }

    tempBIRarray = new double [tab->nCell];
    tempFreqarray = new long [tab->nCell];
    //initialize array
    for (i=0; i<tab->nCell; i++){
    	tempBIRarray[i] = 0;
	tempFreqarray[i] = tab->Cell[i];
    }
    int recnr = 0;
    int res;
    if ((!m_InFileIsFixedFormat)&&(m_IgnoreFirstLine)){
	res = ReadMicroRecord(fdread, str);
    }

    while (!feof(fdread)){
	res = ReadMicroRecord(fdread, str);
	if (++recnr % FIREPROGRESS == 0) {
            FireUpdateProgress((int)(ftell(fdread) * 100.0 / m_fSize));  // for progressbar in container
        }
        switch (res) {
            case INFILE_ERROR:
                fclose(fdread);
		delete[] tempBIRarray;
		delete[] tempFreqarray;
		return false;
                break;
            case INFILE_EOF:
                goto oke;
                break;
            case INFILE_OKE:
//		if ((!m_InFileIsFixedFormat)&&(m_IgnoreFirstLine)&&(recnr == 1)) {
//			continue;
//		}
//		else {
                FillBIRArray(*tab,tempBIRarray,str);
		break;
//		}
        }
    }

    oke:
    fclose(fdread);
    // Now sort the stuff
    QuickSortBIRFreqArray(tempBIRarray, tempFreqarray,0,tab->nCell-1);

    long tempUnsafe = 0;
    for(i=0; i<tab->nCell; i++){
	tempUnsafe = tempUnsafe + tempFreqarray[i];
	if (tempUnsafe > nUnsafe){
            break;
	}
    }

    if ((i == tab->nCell) || (i==0)){
	delete[] tempBIRarray;
	delete[] tempFreqarray;
	return false;
    }
    BIRres = tempBIRarray[i-1];
    *BIRResult = BIRres;

    delete[] tempBIRarray;
    delete[] tempFreqarray;
    return true;
}


bool CMuArgCtrl::FillBIRArray(CTable &tab, double *BIRarray, char *record)
{
    if (!ComputeVarIndices(record)) return false;
    long CellNr=0;
    long j;
    double v;
    long freq;
    double weight;
    bool  HasMissing = false;
    int DimNr[MAXDIM];
    for (j =0; j<tab.nDim; j++){
	CellNr *= tab.SizeDim[j];
	CellNr += m_var[tab.Varnr[j]].TableIndex;
	DimNr[j] = m_var[tab.Varnr[j]].TableIndex;
	if (m_var[tab.Varnr[j]].TableIsMissing){
            HasMissing = true;
	}
    }
    assert(tab.GetCellNr(DimNr) == CellNr);
    bool temp;
    freq = tab.Cell[CellNr];
    if (freq >0 ) {
	if (AddMissing(tab,DimNr,freq,weight, HasMissing)) {
            temp = BaseIndividualRisk(freq,weight,&v);
            assert(temp);
            BIRarray[CellNr] = v;
	}
    }
    return true;
}


void CMuArgCtrl::QuickSortBIRFreqArray(double * BIR, long *Freq, int first, int last)
{
    int i, j;
    double mid, BIRtemp;
    long Freqtemp;
    assert(first >= 0 && last >= first);

    do{
        i = first;
	j = last;
	mid = (BIR[(i + j) / 2]);
        do{
            while (BIR[i] < mid) i++;
            while (BIR[j] > mid) j--;
            if (i < j){
                BIRtemp = BIR[i];
                BIR[i] = BIR[j];
		BIR[j] = BIRtemp;
		Freqtemp = Freq[i];
		Freq[i] = Freq[j];
		Freq[j] = Freqtemp;
            } else {
		if (i == j){
                    i++;
                    j--;
		}
		break;
            }
	} while (++i <= --j);

	if (j - first < last - i){
            if (j > first) {
                QuickSortBIRFreqArray(BIR,Freq, first, j);
            }
            first = i;
	} else {
            if (i < last){
                QuickSortBIRFreqArray(BIR,Freq, i, last);
            }
            last = j;
        }
    } while (first < last);
}
/*
double CMuArgCtrl::FindBIRforNumIterations(double BIR0, long NumIter,
																	  long nUnsafe, double *BIRArray,
																	  long *FreqArray, CTable &t)
{
	long tempUnsafe, CellNr, mid, first, last, i, j;
	double tempBIR;
	tempUnsafe = 0;

	for (i=0; i< t.nCell; i++)	{
		if ((BIRArray[i] <= BIR0) && (BIRArray[i+1]>BIR0)) {
			CellNr = i;
			break;
		}
	}

	for (i=0; i<=CellNr; i++)	{
		tempUnsafe = tempUnsafe + FreqArray[i];
	}

	if (nUnsafe == tempUnsafe)	{
		return BIRArray[CellNr];

	}

	if (nUnsafe < tempUnsafe)	{
		first = 0; last = CellNr;
	}

	if (nUnsafe > tempUnsafe) {
		first = CellNr; last = t.nCell;
	}

	for (i=0; i<NumIter; i++)	{
		mid = (first +last)/2;
		tempBIR = BIRArray[mid];
		tempUnsafe =0;
		for (j=0; j<mid; j++)	{
			tempUnsafe = tempUnsafe + FreqArray[j];
		}
		if (nUnsafe == tempUnsafe)	{
			break;
		}
		if (nUnsafe < tempUnsafe)	{
			last = mid;
		}
		if (nUnsafe > tempUnsafe) {
			first = mid;
		}
	}
	return tempBIR;
}

*/



bool CMuArgCtrl::CalculateBHRFreq(long TableIndex, bool UseNumOfHH, long nUnsafeHH, long nUnsafeRec, double *ResBHR, long *ErrCode)
{
    long ind = TableIndex -1;
    long i, BIRCounter=0;
    CTable *tab;
    double *tempBHRarray;
    long *tempFreqarray;
    long tempUnsafeHH=0;
    long tempUnsafeRec =0;
    double BHRRes;
    if ((ind < 0) || (ind >= m_ntab)){
	*ErrCode = NOTABLES;
	return false;
    }

    tab = &(m_tab[ind]);
    if (!tab->IsBIR){
	*ErrCode = NOBIRTAB;
	return false;
    }

    if (m_lNumberOfHH==0){
	*ErrCode = NOHOUSEHOLDS;
	return false;
    }

    for (i=0; i<m_ntab; i++){
	if (i== ind){
            break;
        }
	if (m_tab[i].IsBIR){
            BIRCounter++;
	}
    }

    tempBHRarray = new double [m_lNumberOfHH];
    for (i=0; i<m_lNumberOfHH; i++){
	tempBHRarray [i] = m_HH[i].m_dBHR[BIRCounter];
    }
    if (UseNumOfHH){
	QuickSortDoubleArray(tempBHRarray, 0, m_lNumberOfHH-1);
	for (i=0; i<m_lNumberOfHH; i++){
            tempUnsafeHH++;
            if (tempUnsafeHH > nUnsafeHH){
		break;
            }
	}

	if ((i == m_lNumberOfHH) || (i==0)){
            delete[] tempBHRarray;
            return false;
	}
	BHRRes = tempBHRarray[i-1];
	*ResBHR = BHRRes;
    }
    else{
	tempFreqarray = new long [m_lNumberOfHH];
	for (i=0; i< m_lNumberOfHH; i++){
            tempFreqarray[i] = m_HH[i].m_lNumberofMembers;
	}

        QuickSortBIRFreqArray(tempBHRarray, tempFreqarray, 0, m_lNumberOfHH-1);
	for (i=0; i<m_lNumberOfHH; i++){
            tempUnsafeRec = tempUnsafeRec + tempFreqarray[i];
            if (tempUnsafeRec > nUnsafeRec){
		break;
            }
	}
	if ((i == m_lNumberOfHH) || (i==0)){
            delete[] tempBHRarray;
            delete[] tempFreqarray;
            return false;
	}
	BHRRes = tempBHRarray[i-1];
	*ResBHR = BHRRes;
    }

    delete[] tempBHRarray;
    return true;
}

bool CMuArgCtrl::MakeAnonFile(std::string FileName, long nVar, long* VarIndexes, std::string separator, long* ErrorCode)
{
    std::string sFileName;
    sFileName = FileName;
    FILE *fd_in, *fd_out;
    char str[MAXRECORDLENGTH];
    char orgstr[MAXRECORDLENGTH];
    int i, j, recnr;

    if (m_nvar == 0 || m_ntab == 0 || m_fname[0] == 0) {
	return false;
    }

    // open input
    fd_in = fopen(m_fname, "r");
    if (fd_in == 0) {
	return false;
    }

    // open output
    fd_out = fopen(sFileName.c_str(), "w");
    if (fd_out == 0) {
	return false;
    }
    
    // initialize nSuppress in variables
    for (i = 0; i < m_nvar; i++) {
        m_var[i].nSuppress = 0;
    }
    
    MakeFreeRecordDescription(HHIDENT_NO);

    recnr = 0;
    int ReadCode;

    while (1) {
        if (ReadCode = ReadMicroRecord(fd_in, str), ReadCode != INFILE_OKE) {
            assert(ReadCode != INFILE_ERROR);
            break;  // error (should not be possible) or eof
        }
        recnr++;
        if (recnr % FIREPROGRESS == 0) {
            FireUpdateProgress((int)(recnr * 100.0 / m_nRecFile) );  // for progressbar in container
        }

        if ((!m_InFileIsFixedFormat)&&(m_IgnoreFirstLine)&&(recnr == 1)) {
            if (!m_FirstLine.empty()) {
                fprintf(fd_out, "%s\n", m_FirstLine.c_str());
            }
            continue;
        }
        strcpy((char *)orgstr,(char *)str);
        // compute (recode)indices out of alfanumerical code for every categorical variable
        if (!ComputeVarIndices(str)) return false;
        // now replace the string with some stuff
        WriteAnonInfo(fd_out, str, recnr, nVar, VarIndexes, separator, orgstr);
    }

    fclose(fd_in);
    fclose(fd_out);
    FireUpdateProgress(100);  // for progressbar in container

    return true;

error:
    fclose(fd_in);
    fclose(fd_out);

    return false;
}

bool CMuArgCtrl::WriteAnonInfo(FILE *fd_out, char *record, long recnr, long nVar, long* VarIndexes, std::string separator, char *origrecord)
{
    char str[MAXRECORDLENGTH];
    int i, iVar;
    CVarList *v = &(m_varlist[0]);// to suppress warning
    CChSafeVarInfo *objVarInfo;
    std::string stempstr;
    CVariable *tempvar;
    char connumstr[MAXCODEWIDTH];
    std::string OutString,TempString,tempcode;
    std::string InString;
    InString = record;
    OutString = "";

    long lfilenum,lArrIndex;

    // Here you have to see if the variable is in one of the objects and then replace it.
    for (i = 0; i < m_nvarpos; i++) {
        v = &(m_varlist[i]);
        if (IsInVarIndexes(v->VarIndex + 1,nVar,VarIndexes)){
            assert(v->d_npos >= 0);
            if(v->d_npos == 0) continue;
            
            iVar = v->VarIndex;
            // unused positions
            if (iVar < 0) { return false; }

            CVariable *var = &m_var[iVar];
                
            // a "normal" variable
            if (!var->HasRecode) {
            // no recode variable
                if (m_InFileIsFixedFormat){
                    tempcode = InString.substr(var->bPos,var->nPos);
                    OutString = OutString + separator + tempcode;
                }
                else {
                    if (ReadVariableFreeFormat(record,i,&(tempcode))) {
                        OutString = OutString + separator + tempcode;
                    }
                }
            } else {
                // recode variable
                int t = var->TableIndex;
                assert(t >= 0 && t < var->Recode.nCode);
                TempString = var->Recode.sCode[t];
                OutString = OutString + separator + TempString;
            }
            OutString = trimright(OutString);
        }
    }
    
    fprintf(fd_out,"%s\n", OutString.substr(1,OutString.size() - 1).c_str());
    
    return true;
}

bool CMuArgCtrl::IsInVarIndexes(long index, long nVar, long* VarIndexes){
    // VarIndexes contains 1-based indexes of k-anonymity variables
    // index 
    long i;
    for (i=0;i<nVar;i++){
        if (index == VarIndexes[i])
            return true;
    }
    return false;
}

