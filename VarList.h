#if !defined VarList_h
#define VarList_h


class CVarList {
public:
	int s_bpos;             // start position source record, first byte 0
	int s_npos;             // number of position source record
	int VarIndex;           // index variable, -1 for untouched positions
	int d_bpos;             // start position destination record, first byte 0
	int d_npos;             // number of positions in destinationrecord
};

#endif
