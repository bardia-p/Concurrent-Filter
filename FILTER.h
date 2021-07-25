#define ARRAY_SIZE 7

struct shared_use_st {
	int arrayDone;
	int didSwap[3];
	char B[ARRAY_SIZE];
};

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};
