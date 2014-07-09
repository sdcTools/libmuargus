#include "Household.h"
#include <string.h>

bool CHousehold::CalculateBHR(long NumberofBIR)
{
    long i,j,k;
    double tempsum, tempproduct;

    if (m_lNumberofMembers <= 0) {
	return false;
    }
    tempsum = 0;
    for (k=0; k<NumberofBIR; k++) {
	for (i=0; i<m_lNumberofMembers; i++){
            tempproduct = 1;
            for (j=0; j<i; j++)	{
		tempproduct = tempproduct*(1-m_dBIRarray[(k*m_lNumberofMembers)+j]);
            }
            tempproduct = tempproduct*m_dBIRarray[(k*m_lNumberofMembers)+i];
            tempsum = tempsum+tempproduct;
	}
	//first clear the BIRarray
	m_dBHR[k] = tempsum;
	tempsum = 0;
	tempproduct = 1;
    }
    delete[] m_dBIRarray;
    m_dBIRarray = 0;
    return true;
}

bool CHousehold::PrepareHouseholdBIR(long NumberofBIR)
{
    if ((m_lNumberofMembers <1)|| (NumberofBIR<=0))	{
        return false;
    }
    m_dBIRarray = new double [m_lNumberofMembers*NumberofBIR];
    if (m_dBIRarray == 0) {
        return false;
    }
    memset(m_dBIRarray, 0, m_lNumberofMembers *NumberofBIR* sizeof(double) );
    return true;
}

bool CHousehold::PrepareHouseholdBHR(long NumberofBIR)
{
    if (NumberofBIR<= 0) {
	return false;
    }
    m_dBHR = new double [NumberofBIR];
    if (m_dBHR == 0) {
	return false;
    }
    memset(m_dBHR, 0, NumberofBIR* sizeof(double) );
    return true;
}

void CHousehold::SaveBIR(double *BIRarray, long NumberOfBIR, long MemberNumber)
{
    long i;
    for (i=0; i<NumberOfBIR; i++)	{
	m_dBIRarray[i*m_lNumberofMembers + MemberNumber] = BIRarray[i];
    }
}