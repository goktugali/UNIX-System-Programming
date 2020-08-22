/**
* CSE344 SYSTEMS PROGRAMMING
* HOMEWORK 1
* GOKTUG ALI AKIN | 161044018
*
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> 
#include <signal.h> 

#include "utils.h"

extern int errno;

/** parse and check the command line arguments **/
void run_programA(int argc, char *argv[]);
void terminate_programA();
void init_programA(const char* outputPathA);

void int_handler()
{
	terminate_programA();
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{	

	internal_file_test();

	/*listen for the incoming Ctrl + C 
	* for terminating the program properly 
	*/
	signal(SIGINT,int_handler);

	run_programA(argc,argv);
	terminate_programA();
	return 0;
}

void init_programA(const char* outputPathA)
{

	file_stat status = file_status();

	if(status.B_CNT == -1 && (status.A_CNT == -1 || status.A_CNT == 0)){

		int fd = open(outputPathA,O_WRONLY);

		if(fd<0){
			fprintf(stderr, "Error opening file : %s\nProgram Terminated\n", outputPathA);
			terminate_programA();
			exit(EXIT_FAILURE);
		} 

		struct flock lock; // lock for outputPathA
		memset(&lock,0,sizeof(lock));
		lock.l_type = F_WRLCK;
		fcntl(fd,F_SETLKW,&lock);

		fd = open(outputPathA,O_WRONLY | O_TRUNC);

		lock.l_type = F_UNLCK;
		fcntl(fd,F_SETLKW,&lock);
		close(fd);
	}

	if(status.A_CNT == -1){

		status.A_CNT = 1;
		update_file_status(status);
		return;
	}

	status.A_CNT = status.A_CNT + 1;
	update_file_status(status);
}


void run_programA(int argc, char *argv[])
{
	
	/* PROGRAM VARIABLES */
	char inputPathA[MAX_FILE_SIZE];
	char outputPathA[MAX_FILE_SIZE];
	int sleep_time;

	if(parse_args(inputPathA,outputPathA,&sleep_time,argc,argv))
		exit(EXIT_FAILURE);

	
	int input_fd = open(inputPathA, O_RDONLY);
	int output_fd = open(outputPathA,O_RDWR);

	if(input_fd<0){
		fprintf(stderr, "Error opening file : %s\nProgram Terminated\n", inputPathA);
		terminate_programA();
		exit(EXIT_FAILURE);
	}

	if(output_fd<0){
		fprintf(stderr, "Error opening file : %s\nProgram Terminated\n", outputPathA);
		terminate_programA();
		exit(EXIT_FAILURE);
	}  


	/* init ipc file */
	init_programA(outputPathA);

	char * buff = (char*)calloc(BUFF_SIZE+1,sizeof(char));
	int size;

	struct flock lock;
	/** init flock **/
	memset(&lock,0,sizeof(lock));

	int read_ctr = 0;
	int line_size = 0;

	printf("# Process %d %s >> %s with sleep time %ds.\nProcess started..\n\n", getpid(),inputPathA,outputPathA,sleep_time);

	while(read(input_fd,buff,BUFF_SIZE) == BUFF_SIZE)
	{	
		/* get complex numbers data */
		complex_num * numbers = str_to_complex_arr(buff,BUFF_SIZE);
		char * res_str = complex_arr_to_string(numbers,BUFF_SIZE / 2,'i');

		/* lock the file and write */
		lock.l_type = F_WRLCK;
		fcntl(output_fd,F_SETLKW,&lock);

		char str[2];
		lseek(output_fd,0,SEEK_SET);
		while((size = read(output_fd,str,1)))
		{	
			str[1] = '\0';
			if(strcmp(str,"") == 0)
				break;
			++line_size;
		}
		
		write(output_fd,res_str,strlen(res_str));
		//write(output_fd,buff,BUFF_SIZE);
		write(output_fd,"\n",1);
		lock.l_type = F_UNLCK;
		fcntl(output_fd,F_SETLKW,&lock);

		sleep(sleep_time);

		/* free heap allocated data */
		free(numbers);
		free(res_str);
		++read_ctr;
		line_size=0;
		lseek(input_fd,1,SEEK_CUR);
	}

	close(input_fd);
	close(output_fd);

	free(buff);
	printf("%d Total bytes (%d complex numbers) readed..\nNo more bytes to read...\nProcess %d finished\n",read_ctr*32,read_ctr*16,getpid());

}
/**
* not directly kill the program!
* decrements counter in the ipc file
*/
void terminate_programA()
{

	/* ipc file format 
	* AX 
	* BY 
	* where x is the number of intances of A.
	* where y is the number of instances of B.
	*/

	/* get the ipc file status */	
	file_stat status = file_status();
	if(status.A_CNT <= 0)
		return;
	/* decrement the number of instances */
	status.A_CNT = status.A_CNT - 1;
	
	/* update the ipc file */
	update_file_status(status);
}