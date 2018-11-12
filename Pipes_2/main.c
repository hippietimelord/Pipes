/*
Assignment 1 - ITS60503- Operating Systems
Student Name : Catherine Labial John
Student ID   : 0325773
Student Name : Afsana Islam
Student ID   : 0326710
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define INPUT_BUFFER 1024
#define MAX_LINES 16

/*Read File and Write File Pseudo Function */
void *read();
void *write();

/*Read and Write File pointers*/
FILE *Rptr;
FILE *Wptr;

/*Shared buffer pointer*/
char *shared_buffer[MAX_LINES]={0};

/*Mutex for Read and Write Threads*/
pthread_mutex_t mutexREAD = PTHREAD_MUTEX_INITIALIZER;	
pthread_mutex_t mutexWRITE = PTHREAD_MUTEX_INITIALIZER;

int flag = 0; /*Indicator flag for End of file*/
int linesFill = 0; /*Show how many lines in the shared buffer is filled = read()*/
int linesRead= 0;  /*Show how many lines in the shared buffer that is read = write()*/

int main(int argc,char *argv[]){	
	/*If input from User Doesnt follow the format*/
	if(argc != 4){
		printf("Invalid Input format!!!\nFollow the format given below:\n");
		printf("./main <input.txt> <output.txt> <Number Of Threads>\n");
	    return 1;
	}
	
	/*Get input from User ( ./main[0] <input_file>[1] <output_file>[2] <number_of_threads>[3] )*/
	int c1,c2; /*Counter for CREATE threads*/
	int j1,j2; /*Counter for JOIN threads */
	int no_of_threads = atoi(argv[3]); /*Get Number of thread from User input */
	pthread_t R_Thread[no_of_threads]; /*Read Threads*/
	pthread_t W_Thread[no_of_threads]; /*Write Threads*/
	
	/* Read from the input file entered by user*/
	Rptr = fopen(argv[1],"r"); 
	if(!Rptr){
		printf("Unable to open %s\n",argv[1]);
		exit(2);
	}
	
	/*Write to output file entered by user*/
	Wptr = fopen(argv[2],"w"); 
	if(!Wptr){
		printf("Unable to create %s\n",argv[2]);
		exit(2);
	}
	
	if(no_of_threads < 1){
		printf("Cannot manage with less than 1 thread\n");
		exit(3);
	}

	/*Creates Reader thread */
	for(c1=0; c1<no_of_threads; c1++){
		pthread_create(&R_Thread[c1], NULL, &read, NULL);
	}
	
	/*Creates Writer thread */
	for(c2=0; c2<no_of_threads; c2++){
		pthread_create(&W_Thread[c2], NULL, &write, NULL);
	}
	
	/*Join Reader Threads*/
	for(j1 = 0; j1 < no_of_threads; j1++){
		pthread_join(R_Thread[j1], NULL);
    }
    
    /*Join Writer Threads*/
	for(j2 = 0; j2 < no_of_threads; j2++){
		pthread_join(W_Thread[j2], NULL);
	
    }
    
    fclose(Rptr);
    fclose(Wptr);
    
    printf("\nSuccessfully copied to output file!\n");
    printf("\nNote :\n");
    printf("To check whether the files are identical enter $diff %s %s", argv[1],argv[2]);
    printf("\nThere will be no results if the files are indentical\n\n");
    return 0;
}

void *read(){
	/*File Read Buffer */
	char R_buffer[INPUT_BUFFER];
	
    /*Infinite loop will break when everything has been read and enter into shared buffer */
	for(;;)	{
       pthread_mutex_lock( &mutexREAD ); /*Lock thread for reading*/
	   printf("MutexReadLock\n");
	   /*When file has reached the end of file */
       if(feof(Rptr)){
       	 flag = 1;
       	 pthread_mutex_unlock(&mutexREAD);
       	 pthread_exit(0);
       	 break;
	   } 
		for(;;){
			if(!shared_buffer[linesFill])
				break;
		}
        /* if there are still lines to fill in the shared buffer */
	    if(linesFill != 16) {
			if(fgets(R_buffer, INPUT_BUFFER, Rptr)) {	
	            /*allocate memory for read buffer and copy string into shared buffer*/
				shared_buffer[linesFill] = (char*)malloc(strlen(R_buffer) + 1);		
				strcpy(shared_buffer[linesFill], R_buffer);		
				/*Reset the number in shared buffer*/
				if(linesFill < 15){
					linesFill++;
				}else{
					linesFill = 0;
				}
			}
		}
		printf("\tMutexReadUnlock\n");
		pthread_mutex_unlock(&mutexREAD);/*Unlock thread*/
    }
	
}

void *write(){	
	for(;;)	{
       pthread_mutex_lock( &mutexWRITE ); /*Lock the thread for writing*/
       printf("\t\tMutexWriteLock\n");
       
	   for(;;){
			if(shared_buffer[linesFill] || flag == 1)
				break;
		}
		/*Infinite loop will break when all contents are read from shared buffer and copied into output file*/
       	if(shared_buffer[linesRead]) {
			fputs(shared_buffer[linesRead], Wptr); /*Write current line to outputfile */
			free(shared_buffer[linesRead]);	/*free memory space*/
			shared_buffer[linesRead] = NULL;		
			/*resets number in shared buffer*/
			if(linesRead < 15){
				linesRead++;
		    }else{
				linesRead = 0;
			}
		}
		/*When the current line is empty and flag returns true it unlocks & terminates the thread*/
		else if(!shared_buffer[linesRead] && flag == 1) {
			pthread_mutex_unlock(&mutexWRITE);				
			pthread_exit(0);		
			break;
		}
		printf("\t\t\tMutexWriteLock\n");
		pthread_mutex_unlock(&mutexWRITE);	/*Unlock current thread*/
    }	
}




