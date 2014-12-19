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

#if !defined Household_h
#define Household_h

class CHousehold
{
public:

	CHousehold()
	{
            m_lNumberofMembers = 0;
            m_dBIRarray = 0;
            m_dBHR = 0;

	}
	~CHousehold()
	{
            if (m_dBHR != 0)
            {
		delete[] m_dBHR;
            }
            if (m_dBIRarray != 0)
            {
            delete[] m_dBIRarray;
            }
            m_dBIRarray = 0;
	}
        
	long m_lNumberofMembers;
	double *m_dBIRarray;
	double *m_dBHR;

	bool CalculateBHR(long NumberofBIR);
	bool PrepareHouseholdBIR(long NumberofBIR);
	bool PrepareHouseholdBHR(long NumberofBIR);
	void SaveBIR(double * BIRarray, long NumberOfBIR, long MemberNumber);



};
#endif 
