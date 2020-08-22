/**
* CSE344 SYSTEMS PROGRAMMING
* HOMEWORK 1
* GOKTUG ALI AKIN | 161044018
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "utils.h"
#include "fft.h"

extern int errno;

void update_file(int line_no,int read_fd,int temp_fd,char * inputPathB);
void run_programB(int argc,char *argv[]);
void init_programB(const char * inputPathB,const char * outputPathB);
void terminate_programB();


void int_handler()
{
	terminate_programB();
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{	

	internal_file_test();
	/*listen for the incoming Ctrl + C 
	* for terminating the program properly 
	*/
	signal(SIGINT,int_handler);
		
	run_programB(argc,argv);
	terminate_programB();
	return 0;
}

void init_programB(const char * inputPathB,const char * outputPathB)
{

	file_stat status = file_status();

	if(status.B_CNT == -1)
	{	
		if(status.A_CNT == -1 || status.A_CNT == 0){
			/** init inputPathB **/
			int fd = open(inputPathB,O_WRONLY);

			if(fd<0){
				fprintf(stderr, "Error occured. File %s cannot opened\nExiting...\n", inputPathB);
				exit(EXIT_FAILURE);
			}

			struct flock lock; // lock for inputPathB
			memset(&lock,0,sizeof(lock));
			lock.l_type = F_WRLCK;
			fcntl(fd,F_SETLKW,&lock);

			fd = open(inputPathB,O_WRONLY | O_TRUNC); 
			lock.l_type = F_UNLCK;
			fcntl(fd,F_SETLKW,&lock);
			close(fd);
			/** init inputPathB **/
		}

		/** init outputPathB **/
		int fd2 = open(outputPathB,O_WRONLY);

		if(fd2<0){
			fprintf(stderr, "Error occured. File %s cannot opened\nExiting...\n", outputPathB);
			exit(EXIT_FAILURE);
		}

		struct flock lock2; // lock for inputPathB
		memset(&lock2,0,sizeof(lock2));
		lock2.l_type = F_WRLCK;
		fcntl(fd2,F_SETLKW,&lock2);

		fd2 = open(outputPathB,O_WRONLY | O_TRUNC); 
		lock2.l_type = F_UNLCK;
		fcntl(fd2,F_SETLKW,&lock2);
		close(fd2);
		/** init outputPathB **/

		if(status.A_CNT <= 0)
			status.A_CNT = -1;
		status.B_CNT = 1;
		update_file_status(status);
		return;
	}

	status.B_CNT = status.B_CNT + 1;
	update_file_status(status);
	return;
}

void run_programB(int argc,char *argv[])
{
	
	/* PROGRAM VARIABLES */
	char inputPathB[MAX_FILE_SIZE];
	char outputPathB[MAX_FILE_SIZE];
	int sleep_time;

	if(parse_args(inputPathB,outputPathB,&sleep_time,argc,argv))
		exit(EXIT_FAILURE);
	

	int read_fd = open(inputPathB,O_RDWR);
	int temp_fd = open(TEMP_FILE,O_RDWR);
	int outb_fd = open(outputPathB,O_RDWR);

	if(read_fd<0){
		fprintf(stderr, "Error opening file : %s\nProgram Terminated\n", inputPathB);
		terminate_programB();
		exit(EXIT_FAILURE);
	}

	if(temp_fd<0){
		fprintf(stderr, "Error opening file : %s\nProgram Terminated\n", TEMP_FILE);
		terminate_programB();
		exit(EXIT_FAILURE);
	}

	if(outb_fd<0){
		fprintf(stderr, "Error opening file : %s\nProgram Terminated\n", outputPathB);
		terminate_programB();
		exit(EXIT_FAILURE);
	}


	/* init ipc file */
	init_programB(inputPathB,outputPathB);

	printf("# Process %d %s >> %s with sleep time %ds.\nProcess started..\n\n", getpid(),inputPathB,outputPathB,sleep_time);
	file_stat status = file_status();
	if(status.A_CNT == -1)
		printf("(!) No instance of programA running now, programB will wait until (Ctrl + C) hit as default...\n");

	/* initialize lock structs */
	struct flock lock; // lock for inputPathB
	struct flock lock2; // lock for TEMP_FILE
	struct flock lock3; // lock for outputPathB
	memset(&lock,0,sizeof(lock));
	memset(&lock2,0,sizeof(lock2));
	memset(&lock3,0,sizeof(lock3));


	int size;
	int line_size = 1;
	int cur_file_pos = 0;
	int line_no = 1;

	while(TRUE)
	{	

		/*** LOCK **/
		lock.l_type = F_WRLCK;
		fcntl(read_fd,F_SETLKW,&lock);

		lock2.l_type = F_WRLCK;
		fcntl(temp_fd,F_SETLKW,&lock2);

		lock3.l_type = F_WRLCK;
		fcntl(outb_fd,F_SETLKW,&lock3);
		
		char str1[2];

		lseek(read_fd,0,SEEK_SET);
		/* get the line number */
		while((size = read(read_fd,str1,1)))
		{	
			str1[1] = '\0';
			if(strcmp(str1,"\n") != 0)
				break;	
			++line_no;
		}

		lseek(read_fd,cur_file_pos,SEEK_SET);
		char str[2];
		while((size = read(read_fd,str,1)))
		{	
			str[1] = '\0';

			if(strcmp(str,"\n") == 0)
				break;	
				
			++line_size;
		}
		
		if(size != 0){
			lseek(read_fd,cur_file_pos,SEEK_SET);
			char * line = (char*)calloc(line_size,sizeof(char));
			read(read_fd,line,line_size);
			if(strcmp(line,"\n") != 0){

				line[line_size-1] = '\0';
				
				/* process the line */
				int out_size;
				char out_str[2];
				while((out_size = read(outb_fd,out_str,1)));
				
				complex_num * numbers = strline_to_complex_arr(line);
				FFT(numbers,BUFF_SIZE / 2, SAMPLING_STEPS);
				char * res_line = complex_arr_to_string(numbers,BUFF_SIZE / 2,'d');
		
				write(outb_fd,res_line,strlen(res_line));
				//write(outb_fd,line,line_size-1);
				write(outb_fd,"\n",1);
				
				/* update the file common file, inputPathB*/
				lseek(read_fd,0,SEEK_SET);
				update_file(line_no,read_fd,temp_fd,inputPathB);

				cur_file_pos = 0;

				/* free allocated memory */
				free(numbers);
				free(res_line);
			}
			else
				cur_file_pos += line_size;
			free(line);
			
		}
		
		lseek(read_fd,cur_file_pos,SEEK_SET);

		lock.l_type = F_UNLCK;
		fcntl(read_fd,F_SETLKW,&lock);

		lock2.l_type = F_UNLCK;
		fcntl(temp_fd,F_SETLKW,&lock2);

		lock3.l_type = F_UNLCK;
		fcntl(outb_fd,F_SETLKW,&lock3);

		if(size == 0){
			file_stat ipc_status = file_status();
			if(ipc_status.A_CNT == 0)
				break;
			else
				sleep(sleep_time);	
			
		}
		line_no = 1;
		line_size = 1;
	}

	close(read_fd);
	close(temp_fd);
}

/**
* update the IPC file.
* for terminate the program properly
*/
void terminate_programB()
{
	/* ipc file format 
	* AX 
	* BY 
	* where x is the number of intances of A.
	* where y is the number of instances of B.
	*/

	/* get the ipc file status */	
	file_stat status = file_status();
	if(status.B_CNT == 0)
		return;
	/* decrement the number of instances */
	status.B_CNT = status.B_CNT - 1;

	if(status.B_CNT == 0)
		status.B_CNT = -1;
	
	/* update the ipc file */
	update_file_status(status);
}

/**
* update the common file between programA and programB (inputpathB = outputpathA)
* by replacing the processed line with '\n'
*/

void update_file(int line_no,int read_fd,int temp_fd,char * inputPathB)
{

	int size;
	char str[2];
	int line_size = 1;
	int cur_file_pos = 0;
	int line_no_ctr = 1;

	temp_fd = open(TEMP_FILE,O_RDWR | O_TRUNC);
	
	while(TRUE)
	{	
		while((size = read(read_fd,str,1)))
		{			
			str[1] = '\0';
			if(strcmp(str,"\n") == 0)
				break;
			++line_size;
		}
		
		lseek(read_fd,cur_file_pos,SEEK_SET);
		char * line = (char*)calloc(line_size,sizeof(char));
		read(read_fd,line,line_size);

		
		if(strcmp(line,"\n") != 0)
			line[line_size-1] = '\0';

		if(line_no != line_no_ctr)
			write(temp_fd,line,line_size-1);
			
	
		free(line);
		cur_file_pos += line_size;
		lseek(read_fd,cur_file_pos,SEEK_SET);
		line_size = 1;

		if(size==0)
			break;
		
		write(temp_fd,"\n",1);
		
		++line_no_ctr;
	}
	
	lseek(read_fd,0,SEEK_SET);
	lseek(temp_fd,0,SEEK_SET);
	
	line_size = 1;
	cur_file_pos = 0;
	
	read_fd = open(inputPathB,O_RDWR | O_TRUNC);
	
	while(1)
	{	
		while((size = read(temp_fd,str,1)))
		{	
			str[1] = '\0';
			if(strcmp(str,"\n") == 0)
				break;
			++line_size;	
		}
	
		lseek(temp_fd,cur_file_pos,SEEK_SET);
		char * line;
		line = (char*)calloc(line_size,sizeof(char));
		read(temp_fd,line,line_size);
		if(strcmp(line,"\n") != 0){
			line[line_size-1] = '\0';
			write(read_fd,line,line_size-1);
		}
			
		free(line);
		cur_file_pos += line_size;
		lseek(temp_fd,cur_file_pos,SEEK_SET);
		line_size = 1;
			
		if(size==0)
			break;

		write(read_fd,"\n",1);
	}
}
