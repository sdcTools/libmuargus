#if ! defined Constants_h
#define Constants_h

#define MAXCODEWIDTH 100
#define MAXRECORDLENGTH 32000
#define MAXMEMORYUSE 50000000
#define MAXDIM 10
#define SEPARATOR "\r\n"
#define FIREPROGRESS 1000

#define INFILE_ERROR -1
#define INFILE_EOF    0
#define INFILE_OKE    1

#define PRAM_NOT_NA 1000

#define BIR_MINMAX    1
#define BIR_FREQ      2
#define BIR_UNSAFE    3
#define BIR_RATE_RISK 4


// #define TESTBIR

#ifdef _DEBUG
  //#define SHOWTABLE
#endif


enum HHIdentOption {
	HHIDENT_NO,
	HHIDENT_KEEP,
	HHIDENT_CHANGESEQNO,
	HHIDENT_DELETE
};



enum RecodePhase {
	CHECK = 10,
	DESTCODE,
	SRCCODE
};

enum FormToCodes {
        FROMTO_TO = 10,   // -23
        FROMTO_SOLO,      // 34
	FROMTO_FROM,      // 23-
	FROMTO_RANGE      // 23-25
};

// ErrorCodes
enum ErrorCodes {
	FILENOTFOUND = 1000,
	CANTOPENFILE,
	EMPTYFILE,
	WRONGLENGTH,
	RECORDTOOSHORT,
	WRONGRECORD,
        NOVARIABLES,
	NOTABLES,
	NOTENOUGHMEMORY,
        NOTABLEMEMORY, 
	SUBTABLENOSUB,
	SUBTABLEWRONGVAR,
        NODATAFILE,
	PROGRAMERROR,

	// parsing recode
	E_HARD = 2000,
	E_SOFT,
        E_NOVARTABDATA,
	E_LENGTHWRONG,
        E_RANGEWRONG,
	E_VARINDEXWRONG,
	E_EMPTYSPEC,
  
	// codes recode
	R_FROMTOOBIG = 3000,
	R_CODENOTINLIST,
	R_NOSENSE,
	R_MISSING2VALID

};


#endif