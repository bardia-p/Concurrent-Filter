// Bardia Parmoun
// 101143006

#define ARRAY_SIZE 7

struct shared_use_st {
	int isArrayDone[3];
	char B[ARRAY_SIZE];
	int didSwap[3];
};

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};
