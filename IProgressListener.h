#ifndef I_ProgressListener_h
#define I_ProgressListener_h

class IProgressListener
{
public:
	virtual ~IProgressListener() { }
	virtual void UpdateProgress(int perc) = 0;
};

#endif // I_ProgressListener_h
