#define TEXT_SZ 2048

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
struct shared_use_st
{
    int flab;   
    int flba;
    char A_B[TEXT_SZ];
    char B_A[TEXT_SZ];
};

void *Input_Thread(struct shared_use_st *);
void *Output_Thread(struct shared_use_st *);
char message[]="continue";

int main(){
    //Initialization of variables and data structures that we will use
    int running=1;
    void *shared_memory=(void *)0;
    struct shared_use_st *shared_stuff=malloc(sizeof(struct shared_use_st));
    int shmid;
    shmid=shmget((key_t)2223, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
    if (shmid == -1) {
		fprintf(stderr, "shmget failed\n");
		perror("Error:");
		exit(EXIT_FAILURE);
	}
	shared_memory = shmat(shmid, (void *)0, 0);
	if (shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
    shared_stuff=(struct shared_use_st *)shared_memory;
    pthread_t i_thread;
	pthread_t o_thread;
    void* i_result;
	void* o_result;
	int fi;
	int fo;
    char input[BUFSIZ];
    //Start of proccess that is being determined by keyboard
    while (running)
    {
        printf("Activate Proccess B with: BEGIN B\n");
	    fgets(input, BUFSIZ, stdin);
        if (strncmp(input,"BEGIN B",7)==0)
        {
            running=0;
        }
        else if (strncmp(input,"BYE",3)==0)
        {
            printf("programm terminated\n");
            return 0;
        }    
    }
    printf("Initiating Proccess B\n");
    
    //creating threads
    fi=pthread_create(&i_thread,NULL, Input_Thread, shared_stuff);
    fo=pthread_create(&o_thread,NULL, Output_Thread, shared_stuff);
    
    //joining threads
    fi = pthread_join(i_thread, &i_result);
	if (fi != 0) {
		perror("Thread join failed");
		exit(EXIT_FAILURE);
	}
    fo = pthread_join(o_thread, &i_result);
	if (fo != 0) {
		perror("Thread join failed");
		exit(EXIT_FAILURE);
	}
    
    //detaching memory segment
    if (shmdt(shared_memory) == -1) {
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
    
    
    free(shared_memory);
    
	exit(EXIT_SUCCESS);
}

void *Input_Thread(struct shared_use_st *shared){
	printf("Input for B Initiated\n");
    
    int running=1;
    char buffin[BUFSIZ];
    while (running)
    {
        if (shared->flba==0)
        {
            fgets(buffin, BUFSIZ, stdin);
            strncpy(shared->B_A, buffin, TEXT_SZ);
            if (strncmp(buffin, "BYE", 3)==0)
            {
                running=0;
            }
            shared->flba=1;
        }
        
    }
    exit(EXIT_SUCCESS);
}

void *Output_Thread(struct shared_use_st *shared){
	printf("Output for B Initiated\n");
    
    int running=1;
    char buffout[BUFSIZ];
    while (running)
    {
        if (shared->flab==1)
        {
            strncpy(buffout,shared->A_B, TEXT_SZ);
            printf("From A:%s\n",buffout);
            if (strncmp(buffout, "BYE", 3)==0)
            {
                running=0;
                printf("BYE\n");
            }
            shared->flab=0;
        }
        
        
    }
    exit(EXIT_SUCCESS);
}