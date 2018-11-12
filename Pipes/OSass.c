/*
Assignment 1 - ITS60503- Operating Systems
Student Name : Afsana Islam
Student ID   : 0326710
Student Name : Catherine Labial John
Student ID   : 0325773
*/

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h> 

#define BUF_INDEX 128
#define P_MISSIONS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/* 
Short Forms Used:
	 P - Parent
	 C - Child 
	 | - Pipeline
*/

/*Group List Variables for msg*/
typedef struct Msg {
	int i;					
	char text[BUF_INDEX];	
} msg;

/*Group List Variables for msg*/
typedef struct record_time {
	char time_st[50]; 	
	long int microscds;	
} record_time;

int main() {
	
	/*File pointers*/
	FILE *Rparent;	
	FILE *Lparent;	
	FILE *child1txt;	
	FILE *child2txt; 
	FILE *child3txt; 
    
    /*File descriptors*/
	int UNpipe1[2];	/*Unamed pipe, P | C1*/
	int UNpipe2[2];	/*C1 | C2*/
	int UNpipe3[2];	/*P | C3*/
	int Rpipe;	/*FIFO (Name pipe) for Reading , C2 | C3*/
	int Wpipe; /*Name pipe for Writing, C2 | C3*/

    /*Message gotten from pipes*/
	char mainText[BUF_INDEX];	
	char text_pipe_1[BUF_INDEX];	
	char text_pipe_2[BUF_INDEX];	
	char text_pipe_3[BUF_INDEX];	
	char text_pipe_4[BUF_INDEX];
	
	/*Storing child process IDs */	
	pid_t c_pid1;	
	pid_t c_pid2;
	pid_t c_pid3;	
	
	/*Other Variables*/
	msg theMsg;
	record_time r_timing;
	time_t cur_tm;
	struct tm *tm = localtime(&cur_tm);
	struct timeval microsecs;
	
	
    /*Creating Unnamed Pipe 1, P | C1*/
	if(pipe(UNpipe1) == -1) {
		perror("Failed to open pipe 1");
		exit(1);
	}

    /*Creating Unnamed Pipe 2, C1 | C2*/
	if(pipe(UNpipe2) == -1) {
		perror("Failed to open pipe 2");
		exit(1);
	}

	/*Creating Unnamed Pipe 3, P | C3*/
	if(pipe(UNpipe3) == -1) {
		perror("Failed to open pipe 3");
		exit(1);
	}

	/*Creating Named Pipe, C2 | C3*/
	unlink("./fifo");						
	if(mkfifo("./fifo", P_MISSIONS))		
		perror("Failed to create './fifo', it may already exist");

    /*Creating the C1 process*/ 
	switch(c_pid1 = fork()) {
		case -1:
			perror("Error! Unable to Fork Child 1 (c_pid1)!");
			exit(2);

		case 0:
	        /*In the process of C1*/
			switch(c_pid2 = fork()) {
				case -1:
					perror("Error! Unable to Fork Child 2 (c_pid2)!");
					exit(3);

				case 0:
					/*In the process of C2*/
					close(UNpipe1[0]);	
					close(UNpipe1[1]);	
					close(UNpipe2[1]);	

					Wpipe = open("./fifo", O_WRONLY); /*Open pipe for writing*/
					if(Wpipe == -1) {
						perror("Unable to open pipe for writing");
						exit(4);
					}
					
                    /*record the process in C2 text*/
					child2txt = fopen("child2.txt", "a+");
					if(!child2txt)
						printf("Error occured with Child 2 text!\n");
					else {
						/*read from pipe2 , write into ./fifo*/
						while(read(UNpipe2[0], text_pipe_2, BUF_INDEX) != 0) {
							char *ch2;		
							msg msg_txt2;
							
                            /*Getting the msg content*/
							ch2 = strtok(text_pipe_2, "\t");	
							msg_txt2.i = atoi(ch2);
							ch2 = strtok(NULL, "\n");		
							strcpy(msg_txt2.text, ch2);
                            
                            /*Getting the time stamp*/
							cur_tm = time(NULL);
							strftime(r_timing.time_st, sizeof(r_timing.time_st), "%d/%m/%Y %H:%M:%S", tm);
							gettimeofday(&microsecs, NULL);
							r_timing.microscds = microsecs.tv_usec;

							if(msg_txt2.i == 2)	/*if index is 2 it will keep in this process*/
								fprintf(child2txt, "%s.%li\t%s\tKEEP\n", r_timing.time_st, r_timing.microscds, msg_txt2.text);
							else {					
								fprintf(child2txt, "%s.%li\t%s\tFORWARD\n", r_timing.time_st, r_timing.microscds, msg_txt2.text);
								strcat(text_pipe_2, "\t");			
								strcat(text_pipe_2, msg_txt2.text);
								write(Wpipe, text_pipe_2, BUF_INDEX);
							}
						}/*End of while*/
					}/*End of else*/
					fclose(child2txt);

					if(unlink("./fifo") == -1)
						perror("Failed to remove './fifo'");
					else
                    /*Unlinked From ./fifo*/
					close(UNpipe2[0]);	
					close(Wpipe);	

					printf("CHILD 2 (pid %d)\n", getpid());
					wait(NULL);	
					exit(0);
					break;

				default:
					/*back to C1*/
					close(UNpipe1[1]);	
					close(UNpipe2[0]);	

					/*Record process in C1*/
					child1txt = fopen("child1.txt", "a+");
					if(!child1txt)
						printf("Error occured with Child 1 text!\n");
					else{
						while(read(UNpipe1[0], text_pipe_1, BUF_INDEX) != 0) { 
							char *ch1;	
							msg msg_txt1;	
                            /*Getting the msg content*/
							ch1 = strtok(text_pipe_1, "\t");		
							msg_txt1.i = atoi(ch1); 	
							ch1 = strtok(NULL, "\n"); 	
							strcpy(msg_txt1.text, ch1);
							
							cur_tm = time(NULL);
							strftime(r_timing.time_st, sizeof(r_timing.time_st), "%d/%m/%Y %H:%M:%S", tm);
							gettimeofday(&microsecs, NULL);
							r_timing.microscds = microsecs.tv_usec;

							if(msg_txt1.i == 1)	/*if index is 1 it will keep in this process*/
								fprintf(child1txt, "%s.%li\t%s\tKEEP\n", r_timing.time_st, r_timing.microscds, msg_txt1.text);
							else {						
								fprintf(child1txt, "%s.%li\t%s\tFORWARD\n", r_timing.time_st, r_timing.microscds, msg_txt1.text);
								strcat(text_pipe_1, "\t");					
								strcat(text_pipe_1, msg_txt1.text);	
								write(UNpipe2[1], text_pipe_1, BUF_INDEX);
							}
						}/*End of while*/
			    	}/*End of else*/

					fclose(child1txt);

					close(UNpipe1[0]);	
					close(UNpipe2[1]);	

					printf("CHILD 1 (pid %d) and CHILD 2 (pid %d)\n", getpid(), c_pid2);
					wait(NULL); 
					exit(0);
			}/*End of inner switch case*/
		
			break;

		default:
			
			close(UNpipe1[0]);	

			
			Rparent = fopen("msg.txt", "r");
			if(!Rparent)
				printf("File does not exist !\n");
			else{
				while(fgets(mainText, BUF_INDEX, Rparent))
					write(UNpipe1[1], mainText, BUF_INDEX);
				rewind(Rparent); 

				
				while(!feof(Rparent)) {
					fscanf(Rparent, "%d\t%[^\n]\n", &theMsg.i, theMsg.text);

					cur_tm = time(NULL);
					strftime(r_timing.time_st, sizeof(r_timing.time_st), "%d/%m/%Y %H:%M:%S", tm);
					gettimeofday(&microsecs, NULL);
					r_timing.microscds = microsecs.tv_usec;

					Lparent = fopen("parent.txt", "a+");
					if(!Lparent)
						printf("Error occured with 'parent.txt'\n");
					else	
						fprintf(Lparent, "%s.%li\t%s\tFORWARD\n", r_timing.time_st, r_timing.microscds, theMsg.text);
					fclose(Lparent);					
				}/*End of while*/
			}/*End of else*/
			
			fclose(Rparent);
			close(UNpipe1[1]);	

			printf("PARENT (pid %d) and CHILD 1 (pid %d)\n", getpid(), c_pid1);

			/* Creating Child3 process */
			switch(c_pid3 = fork()) {
				case -1:
					perror("Error! Unable to Fork Child 3 (c_pid3)");
					exit(3);

				case 0:
				    /*in child 3 process*/
					close(UNpipe3[0]);
					close(UNpipe2[0]);	
					close(UNpipe2[1]);	
					close(UNpipe1[0]);	
					close(UNpipe1[1]);	

					/*Open pipe for writing*/
					Rpipe = open("./fifo", O_RDONLY); 
					if(Rpipe == -1) {
						perror("Cannot open './fifo' for reading");
						exit(4);
					}
		            /*Record in child 3 txt*/
					child3txt = fopen("child3.txt", "a+");
					if(!child3txt)
						printf("Error occured with 'child3.txt'\n");
					else{					
						while(read(Rpipe, text_pipe_3, BUF_INDEX) != 0) {
							char *ch3;		
							msg msg_txt3;	
                             /*Getting the msg content*/
							ch3 = strtok(text_pipe_3, "\t");		
							msg_txt3.i = atoi(ch3);	
							ch3 = strtok(NULL, "\n");			
							strcpy(msg_txt3.text, ch3);

							cur_tm = time(NULL);
							strftime(r_timing.time_st, sizeof(r_timing.time_st), "%d/%m/%Y %H:%M:%S", tm);
							gettimeofday(&microsecs, NULL);
							r_timing.microscds = microsecs.tv_usec;

							if(msg_txt3.i == 3)	
								fprintf(child3txt, "%s.%li\t%s\tKEEP\n", r_timing.time_st, r_timing.microscds, msg_txt3.text);
							else {					
								fprintf(child3txt, "%s.%li\t%s\tFORWARD\n", r_timing.time_st, r_timing.microscds, msg_txt3.text);
								strcat(text_pipe_3, "\t");		
								strcat(text_pipe_3, msg_txt3.text);
								write(UNpipe3[1], text_pipe_3, BUF_INDEX);
							}
						}
				   }
					
					fclose(child3txt);
					
					close(UNpipe3[1]); 	
					close(Rpipe);		

					printf("CHILD 3 (pid %d)\n", getpid());
					wait(NULL);	
					exit(0);
					break;

				default:
					/* Back to parent process */
					close(UNpipe3[1]);	
					close(UNpipe2[0]);
					close(UNpipe2[1]);	
					close(UNpipe1[0]);
					close(UNpipe1[1]);	

					Lparent = fopen("parent.txt", "a+");
					if(!Lparent)
						printf("Error occured with 'parent.txt'\n");
					else{
						while(read(UNpipe3[0], text_pipe_4, BUF_INDEX) != 0) {
							char *ch4;		
							msg msg_txt4;	

							ch4 = strtok(text_pipe_4, "\t");
							msg_txt4.i = atoi(ch4);	
							ch4 = strtok(NULL, "\n");			
							strcpy(msg_txt4.text, ch4);
	
							cur_tm = time(NULL);
							strftime(r_timing.time_st, sizeof(r_timing.time_st), "%d/%m/%Y %H:%M:%S", tm);
							gettimeofday(&microsecs, NULL);
							r_timing.microscds = microsecs.tv_usec;
						
							fprintf(Lparent, "%s.%li\t%s\tKEEP\n", r_timing.time_st, r_timing.microscds, msg_txt4.text);
						}/*End of while*/
				   }/*End of else*/
				   
					fclose(Lparent);
					close(UNpipe3[0]);	
					printf("PARENT (pid %d) and CHILD 3 (pid %d)\n", getpid(), c_pid3);
					wait(NULL);	
					exit(0);
					
			}/*End of default switch case*/

	} /*End of main switch Case*/
	exit(0);
	return 0;
}/*End of main*/
