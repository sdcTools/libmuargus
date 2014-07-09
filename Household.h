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