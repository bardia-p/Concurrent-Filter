// Bardia Parmoun
// 101143006

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/shm.h>
#include <math.h>
#include <sys/sem.h>
#include <ctype.h>

#include "FILTER.h"

static int array_id;
static int swap_id;
static int done_id;

static int set_semvalue(int sem_id, int i);
static void del_semvalue(int sem_id, int i);
static int semaphore_p(int sem_id, int i);
static int semaphore_v(int sem_id, int i);

/* gets two points to char and swaps them */
void swap_values (char* a, char* b){
	char temp = *a;
	*a = *b;
	*b = temp;	
}

/* looks through all the elements of the array to check to see if they are all 1 (the array is filtered) */
int isDone(int array[]){
	for (int i=0; i< ceil(ARRAY_SIZE/3.0); i++){
		if (array[i] == 0){
			return 0;
		}
	}
	return 1;
}

int main ()
{
	pid_t pid;
	
	// creates a shared memory
	void *shared_memory = (void *)0;
	struct shared_use_st *shared_stuff;
	int shmid;

	// connects the memory to the process
	shmid = shmget ((key_t)4580, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
				
	if (shmid == -1){
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}
				
	shared_memory = shmat (shmid, (void *)0, 0);

	//printf("Memory attached at %X\n", (int) shared_memory);

	shared_stuff = (struct shared_use_st *) shared_memory;

	char str[100];
	int debugMode; //keeps track of debugMode which is used to print extra information

	printf("Do you want to run the program in debug mode? Enter 1 for yes\n");
	scanf("%d", &debugMode);

	scanf("%c");

	printf("Initialize the array: separate values by space \n");

	scanf("%[^\n]%*c", str);

	// initializes the array with the input values
	for (int i=0; i<ARRAY_SIZE; i++){
		shared_stuff->B[i] = str[2*i];
	}
		
	// the array of flags is set to 0 meaning the array is not complete
	for (int i=0; i<3; i++){
		shared_stuff ->isArrayDone[i] = 0;
	}
 

	// creating the semaphore (the array of semaphores is the size of the array size)
	// protects access to the main elements of the array
	array_id = semget((key_t)1010, ARRAY_SIZE, 0666 | IPC_CREAT);
	
	for (int i = 0; i<ARRAY_SIZE; i++){
		if (!set_semvalue(array_id,i)){
			fprintf(stderr, "Failed to initialize semaphore\n");
			exit(EXIT_FAILURE);
		}

	}

	// the swap semaphore is used to protect the access to the shared array of didSwap
	// the size of the number of processes
	swap_id = semget((key_t)1020, ceil(ARRAY_SIZE/3.0), 0666 | IPC_CREAT);

	for (int i = 0; i<ceil(ARRAY_SIZE/3.0); i++){
		if (!set_semvalue(swap_id, i)){
			fprintf(stderr, "Failed to initialize semaphore\n");
			exit(EXIT_FAILURE);
		}

	}

	// the done semaphore is used to protect the access to the shared array of isArrayDone
	// the size of the number of the processes
	done_id = semget((key_t)1030, ceil(ARRAY_SIZE/3.0), 0666 | IPC_CREAT);
	
	for (int i = 0; i<ceil(ARRAY_SIZE/3.0); i++){
		if (!set_semvalue(done_id, i)){
			fprintf(stderr, "Failed to initialize semaphore\n");
			exit(EXIT_FAILURE);
		}

	}


	// forks a new child 
	for (int i = 0; i <  ceil(ARRAY_SIZE/3.0); i++){
		pid = fork();
		switch (pid){
			case 0:
				if (debugMode){
					printf("Process: %d\n", i+1);
				}
			
				// creates a shared memory
				void *shared_memory = (void *)0;
				struct shared_use_st *shared_stuff;
				int shmid;

				// connects the memory to the process
				shmid = shmget ((key_t)4580, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
				
				if (shmid == -1){
					fprintf(stderr, "shmget failed\n");
					exit(EXIT_FAILURE);
				}
				
				array_id = semget((key_t)1010, ARRAY_SIZE, 0666 | IPC_CREAT);
				swap_id = semget((key_t)1020, ceil(ARRAY_SIZE/3.0), 0666 | IPC_CREAT);
				done_id = semget((key_t)1030, ceil(ARRAY_SIZE/3.0), 0666 | IPC_CREAT);

				shared_memory = shmat (shmid, (void *)0, 0);
				

				// printf("Memory attached at %X\n", (int) shared_memory);

				shared_stuff = (struct shared_use_st *) shared_memory;

				while (1){
					// the didSwap flag for the given process is set to 0 
					if (!semaphore_p(swap_id, i)) exit(EXIT_FAILURE);
					shared_stuff->didSwap[i] = 0;
					if (!semaphore_v(swap_id, i)) exit(EXIT_FAILURE);

					// checks the ceil(ARRAY/3.0) elements that are allocated to each array (in this case) 
					// if element j is a char and element j-1 is an integer it swaps them	
					for (int j =2*i + ceil(ARRAY_SIZE/3.0)-1; j> 2*i-1; j--){	
						if (shared_stuff->B[j]>=65 && j!=0 && shared_stuff->B[j-1]<65){
							// waits elements j and j - 1
							if (!semaphore_p(array_id, j)) exit(EXIT_FAILURE);
							if (!semaphore_p(array_id, j-1)) exit(EXIT_FAILURE);
							
							// swaps the elements
							swap_values(&shared_stuff->B[j], &shared_stuff->B[j-1]);
							
							// sets the isArrayDone flag to 0 since the array performed a swap operation so it is not done
							if (!semaphore_p(done_id,i)) exit(EXIT_FAILURE);
							shared_stuff->isArrayDone[i] = 0;
							if (!semaphore_v(done_id,i)) exit(EXIT_FAILURE);
							
							if (debugMode){
								printf("P%d swapped %c and %c\n", i+1, shared_stuff->B[j-1], shared_stuff->B[j]);
							}
						
							// the didSwap flag is set to 1							
							if (!semaphore_p(swap_id, i)) exit(EXIT_FAILURE);
							shared_stuff->didSwap[i] = 1;
							if (!semaphore_v(swap_id, i)) exit(EXIT_FAILURE);

							
							if (!semaphore_v(array_id, j)) exit(EXIT_FAILURE);
							if (!semaphore_v(array_id, j-1)) exit(EXIT_FAILURE);
 	
						}
					}
					
					// if the process did not perform any swap
					if (!shared_stuff->didSwap[i]){
						// considers the processs as done
						if (!semaphore_p(done_id,i)) exit(EXIT_FAILURE);
						shared_stuff->isArrayDone[i] = 1;
						if (!semaphore_v(done_id,i)) exit(EXIT_FAILURE);	

						if (debugMode){
							printf("P%d Did not swap\n", i+1); 
						}
					
						
						// checks to see if the other processes are done as well
						if (isDone(shared_stuff->isArrayDone)){
							// detaches the memory
							if (shmdt(shared_memory)==-1){
								fprintf(stderr, "shmdt failed\n");
								exit(EXIT_FAILURE);
							}
						
							// ends the child process
							exit(0);
						}
					}

				}
			case -1:
				perror("fork failed");
				exit(1);
		}
	}
	

	if (pid!=0){
		// parent code	
		// waits for the child processes to finish
		for (int i = 0; i < ceil(ARRAY_SIZE/3.0); i++){
			wait(NULL);		
		}

		if (debugMode){
			printf("Final Result:\n");
		}

	
		printf("[");
		for (int i = 0; i < ARRAY_SIZE; i++){
			printf(" %c,", shared_stuff->B[i]);
		}

		printf("]\n", shared_stuff -> B[ARRAY_SIZE - 1]);

		// deleting semaphores
		del_semvalue(array_id,0);

		del_semvalue(swap_id,0);


		del_semvalue(done_id,0);
		

	
		// detaches the memory
		if (shmdt(shared_memory)==-1){
			fprintf(stderr, "shmdt failed\n");
			exit(EXIT_FAILURE);
		}
		
		// deletes the memory
		if (shmctl(shmid, IPC_RMID, 0) == -1){
			fprintf(stderr, "shmctl(IPC_RMID) failed\n");
			exit(EXIT_FAILURE);
		}
	

		exit(0);
	}		
}

/* sets the semaphore value to 1 for a given index */
static int set_semvalue(int sem_id, int i)
{
	union semun sem_union;
	
	sem_union.val = 1;

	if (semctl(sem_id, i, SETVAL, sem_union) == -1) return(0);
	
	return(1);
}

/* deletes the semaphore */
static void del_semvalue(int sem_id, int i)
{
	union semun sem_union;

	if (semctl(sem_id, i, IPC_RMID, sem_union) == -1){
		fprintf(stderr, "Failed to delete semaphore\n");
	}
}


/* waits the given semaphore with the index */
static int semaphore_p(int sem_id, int i)
{
	struct sembuf sem_b;
	
	sem_b.sem_num = i;
	sem_b.sem_op = -1; /* P() */
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1){
		fprintf(stderr, "semaphore_p failed\n");
		return(0);
	}
	return(1);
}

/* signals the given semaphore with the index */
static int semaphore_v(int sem_id, int i)
{
	struct sembuf sem_b;
	sem_b.sem_num = i;
	sem_b.sem_op = 1; /* V() */
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_v failed\n");
		return(0);
	}
	return(1);
}
