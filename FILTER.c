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

static int p12_id; //keeps track of the shared element between P1 and P2
static int p23_id; //keeps track of the shared element between P2 and P3
static int done_id; //keeps track of the isArrayDone elemen

static int p12_index = ceil(ARRAY_SIZE/3.0) - 1;
static int p23_index = 2 * (ceil(ARRAY_SIZE/3.0) - 1);	
 
static int set_semvalue(int sem_id);
static void del_semvalue(int sem_id);
static int semaphore_p(int sem_id);
static int semaphore_v(int sem_id);

/* gets two points to char and swaps them */
void swap_values (char* a, char* b){
	char temp = *a;
	*a = *b;
	*b = temp;	
}

/* checks to see if the array is done */
int isDone (char array[]){
	int isLetter = 1;
	for (int i = 0; i < ARRAY_SIZE; i++){
		if (array[i]<65){
			isLetter = 0;
		}

		if (!isLetter && array[i]>=65){
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
	shmid = shmget ((key_t)6789, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
				
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
		if (shared_stuff->B[i]<48 || (shared_stuff->B[i]>57 && shared_stuff->B[i]<65) || shared_stuff->B[i]>90){
			printf("There was an error with the input\n");
			exit(0);
		}
	}
		
	// keeps track of whether the array is done
	shared_stuff->arrayDone = 1;
 

	// protects access to the shared element between p1 and p2
	p12_id = semget((key_t)1357,1 , 0666 | IPC_CREAT);
	
	if (!set_semvalue(p12_id)){
		fprintf(stderr, "Failed to initialize semaphore\n");
		exit(EXIT_FAILURE);
	}

	// protects access to the shared element between p2 and p3
	p23_id = semget((key_t)2468, 1, 0666 | IPC_CREAT);
	
	if (!set_semvalue(p23_id)){
		fprintf(stderr, "Failed to initialize semaphore\n");
		exit(EXIT_FAILURE);
	}

	//protects access to arrayDone
	done_id = semget((key_t) 3570, 1, 0666 | IPC_CREAT);

	if (!set_semvalue(done_id)){
		fprintf(stderr, "Failed to initialize semaphore\n");
		exit(EXIT_FAILURE);	
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
				shmid = shmget ((key_t)6789, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
				
				if (shmid == -1){
					fprintf(stderr, "shmget failed\n");
					exit(EXIT_FAILURE);
				}
				
				p12_id = semget((key_t)1357, 1, 0666 | IPC_CREAT);
				p23_id = semget((key_t)2468, 1, 0666 | IPC_CREAT);
				done_id = semget((key_t) 3570, 1, 0666 | IPC_CREAT);
			
				shared_memory = shmat (shmid, (void *)0, 0);
				

				// printf("Memory attached at %X\n", (int) shared_memory);

				shared_stuff = (struct shared_use_st *) shared_memory;
			
				while (1){
					shared_stuff -> didSwap[i] = 0;
				
					// checks the ceil(ARRAY_SIZE/3.0) elements that are allocated to each array (in this case) 
					// if element j is a char and element j-1 is an integer it swaps them	
					for (int j =2*i + ceil(ARRAY_SIZE/3.0)-1; j> 2*i-1 && j > 0; j--){	
						// waits elements j and j - 1
						if (j == p12_index || j - 1 == p12_index){
							if (!semaphore_p(p12_id)) exit(EXIT_FAILURE);
						} else if (j == p23_index || j - 1 == p23_index){
							if (!semaphore_p(p23_id)) exit(EXIT_FAILURE);
						}
							
						if (shared_stuff->B[j]>=65 && shared_stuff->B[j-1]<65){
							// swaps the elements
							if (debugMode){
								printf("P%d swapped %c and %c\n", i+1, shared_stuff->B[j], shared_stuff->B[j-1]);
							}
						
							swap_values(&shared_stuff->B[j], &shared_stuff->B[j-1]);
							
							// sets the didSwap flag to 1
							shared_stuff->didSwap[i] = 1;

							// resets the arrayDone value
							if (!semaphore_p(done_id)) exit(EXIT_FAILURE);
							shared_stuff->arrayDone = 1;
							if (!semaphore_v(done_id)) exit(EXIT_FAILURE);
						}
						// signals the elements
						if (j == p12_index || j - 1 == p12_index){
							if (!semaphore_v(p12_id)) exit(EXIT_FAILURE);
						} else if  (j == p23_index || j - 1 == p23_index){
							if (!semaphore_v(p23_id)) exit(EXIT_FAILURE);
						}
					}
					
					//prints the did not swap message
					if (!shared_stuff->didSwap[i] && shared_stuff->arrayDone % (2*(i+1)+1)!=0){ //avoids printing repeated steps
						// Multiplies the arrayDone value with the prime number of that process
						if (!semaphore_p(done_id)) exit(EXIT_FAILURE);
						shared_stuff->arrayDone *= (2*(i+1) + 1);
						if (!semaphore_v(done_id)) exit(EXIT_FAILURE);

						if (debugMode){
							printf("P%d Did not swap\n", i+1); 
						}		
					}

					// checks to see if the other processes are done as well
					if (shared_stuff->arrayDone % 105 ==0 && isDone(shared_stuff->B)){
						// detaches the memory
						if (shmdt(shared_memory)==-1){
							fprintf(stderr, "shmdt failed\n");
							exit(EXIT_FAILURE);
						}
						
						// ends the child process
						exit(0);
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

		// prints the final array	
		printf("[");
		for (int i = 0; i < ARRAY_SIZE; i++){
			printf(" %c,", shared_stuff->B[i]);
		}

		printf("]\n", shared_stuff -> B[ARRAY_SIZE - 1]);

		// deleting semaphores
		del_semvalue(p12_id);

		del_semvalue(p23_id);
	
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

/* sets the semaphore value to 1 */
static int set_semvalue(int sem_id)
{
	union semun sem_union;
	
	sem_union.val = 1;

	if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
	
	return(1);
}

/* deletes the semaphore */
static void del_semvalue(int sem_id)
{
	union semun sem_union;

	if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1){
		fprintf(stderr, "Failed to delete semaphore\n");
	}
}


/* waits the given semaphore */
static int semaphore_p(int sem_id)
{
	struct sembuf sem_b;
	
	sem_b.sem_num = 0;
	sem_b.sem_op = -1; /* P() */
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1){
		fprintf(stderr, "semaphore_p failed\n");
		return(0);
	}
	return(1);
}

/* signals the given semaphore */
static int semaphore_v(int sem_id)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1; /* V() */
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_v failed\n");
		return(0);
	}
	return(1);
}
