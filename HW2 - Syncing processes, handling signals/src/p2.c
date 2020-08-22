#include <stdio.h>
#include "utils.h"
#include <signal.h>
#include <sys/types.h>

/* 
* explanation :
In the homework PDF ;
Attention: P2 will remove/delete the file denoted by inputPath after P1 is done with it. How will
P2 know whether P1 has finished writing to it? P1 will send SIGUSR1 to P2 with this purpose.

* P2 process will delete temporary file. P2 process will NOT delete inputhpath of process P1. 
* designed like that intentionally.(P1 has finished writing to it) so it is tempfile
* I understand like that.
*/

/* default init size of array that stores mae,mse results. */
#define DEFAULT_POINT_SIZE 1000

extern int errno;
sig_atomic_t process1_finished = 0;


/** local functions **/

/**
* Initialize signal handlers
*/
void init_signals();

/**
* delete the line from the common (temp) file
*/
void update_file(int read_fd,char * inputPathB);

/**
* terminate the program and print info to the screen.
*/
void terminate_program(double * all_maes,double * all_mses,double * all_rmses,int point_size);

/**
* calculate standart devivation.
* @param all_values array that stores values
* @param mean of all_values. If mean param is given as -1, this function calculate mean by itself.
* @param size number of values
*/
double calc_stddev(double * all_values,double mean,int size);

/**
* calculate mean.
* @param all_values array that stores values
* @param size number of values
*/
double calc_mean(double * all_values, int size);

/**
* cleanup gracefully before the exiting.
* closing the files, removing the files, deleting the malloc(ed) memory.
*/
void cleanup();

/** end of local functions **/


/* global variables for cleanup before exit */
int glb_input_fd;
int glb_outpud_fd;
char glb_temp_file[MAX_FILE_SIZE];

/* store a pointer of MAE,MSE,RMSE results array */
double ** glb_all_maes = NULL;
double ** glb_all_mses = NULL;
double ** glb_all_rmses = NULL;


/* SIGUSR1 handler */
void handler1(int signo)
{	
	process1_finished = 1;
}

/* SIGUSR2 handler */
void handler2(int signo)
{
 /* wake up the this program */
}

/* sigterm handler */
void handler_TERM(int signo)
{	
	printf(">> [p2] interrupted by SIGTERM\n");
	cleanup();
	exit(EXIT_FAILURE);
}
/* sigint handler */
void handler_INT(int signo)
{	
	printf(">> [p2] interrupted by SIGINT\n");
	cleanup();
	exit(EXIT_FAILURE);
}

int main(int argc, char*argv[])
{	
	
	/* initialize signals */
	init_signals();

	/* block the sigusr1 and sigusr2 */
	/* theese signals will listen just in the suspend 
	* otherwise, communatioc signal between p1 and p2 can be loss.
	*/
	sigset_t init_mask;
	if(sigemptyset(&init_mask) == -1){
		fprintf(stderr, "Signal mask cannot be initialized now\nExit.\n");
		exit(EXIT_FAILURE);
	}
	if(sigaddset(&init_mask,SIGUSR1) == -1 || sigaddset(&init_mask,SIGUSR2) == -1){
		fprintf(stderr, "Signal mask cannot be initialized now\nExit.\n");
		exit(EXIT_FAILURE);
	}
	
	if(sigprocmask(SIG_BLOCK,&init_mask,NULL) == -1){
		fprintf(stderr, "Failed with error at signal block : %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	printf(">> [p2] process started..\n");

	/* notify the parent (p1) that I am (p2) started */
	/**
	* p2 (ben), start sinyali gondermeden, p1 bana sinyal gondermemeli.
	* SIGUSR2 pending in the parent, SIGUSR1 not.
	*/
	if(kill(getppid(),SIGUSR1)== -1 || kill(getppid(),SIGUSR2) == -1){
		fprintf(stderr, "Failed with error at signal sending : %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	char inputPath[MAX_FILE_SIZE];
	char outputPath[MAX_FILE_SIZE];
	
	if(parse_args(inputPath,outputPath,argc,argv))
		exit(EXIT_FAILURE);

	int input_fd = open(inputPath, O_RDWR);
	int outpud_fd = open(outputPath,O_RDWR | O_TRUNC);

	if(input_fd<0){
		fprintf(stderr, "Error opening file : %s\nProgram Terminated\n", inputPath);
		exit(EXIT_FAILURE);
	}
	if(outpud_fd<0){
		fprintf(stderr, "Error opening file : %s\nProgram Terminated\n", outputPath);
		exit(EXIT_FAILURE);
	}

	/* init global variables */
	glb_input_fd = input_fd;
	glb_outpud_fd = outpud_fd;
	glb_temp_file[0] = '\0';
	strcpy(glb_temp_file,inputPath);

	int size;
	int line_size = 0;

	struct flock lock; // lock for inputPath (temp file)
	memset(&lock,0,sizeof(lock));
	char * line;

	/* create dynamic array to store all MAE values */
	double * all_maes = (double*)calloc(DEFAULT_POINT_SIZE,sizeof(double));
	glb_all_maes = &all_maes;
	/* create dynamic array to store all MSE values */
	double * all_mses = (double*)calloc(DEFAULT_POINT_SIZE,sizeof(double));
	glb_all_mses = &all_mses;
	/* create dynamic array to store all RMSE values */
	double * all_rmses = (double*)calloc(DEFAULT_POINT_SIZE,sizeof(double));
	glb_all_rmses = &all_rmses;

	int current_size = DEFAULT_POINT_SIZE;
	int cur_index = 0;

	/* calculation results as string */
	char mae_str[10],mse_str[10],rmse_str[10];

	/* (ax + b) */
	double a,b;
	double mae,mse,rmse;

	char str1[2];
	while(TRUE)
	{	
		line_size = 0;
		/*** LOCK **/
		lock.l_type = F_WRLCK;
		fcntl(input_fd,F_SETLKW,&lock);

		lseek(input_fd,0,SEEK_SET);
		str1[1] = '\0';

		/* calculate the line size */
		while((size = read(input_fd,str1,1)))
		{
			str1[1] = '\0';
			if(strcmp(str1,"\n")==0)
				break;
			++line_size;
		}
		
		if(line_size > 0){

			lseek(input_fd,0,SEEK_SET);
			line = (char*)calloc(line_size+1,sizeof(char));
			read(input_fd,line,line_size+1);
			line[line_size] = '\0';
			
			/* process the line */
			
			point_2D * point_arr = strline_to_point_arr(line,BUFF_SIZE / 2,&a,&b);
					
			sigset_t block_mask;
			if(sigemptyset(&block_mask) == -1){
				fprintf(stderr, "Signal mask cannot be initialized now\nExit.\n");
				exit(EXIT_FAILURE);
			}
			if(sigaddset(&block_mask,SIGINT) == -1 || sigaddset(&block_mask,SIGSTOP) == -1){
				fprintf(stderr, "Signal mask cannot be initialized now\nExit.\n");
				exit(EXIT_FAILURE);
			}
			
			/* BLOCK SIGINT and SIGSTOP */
			if(sigprocmask(SIG_BLOCK,&block_mask,NULL) == -1){
				fprintf(stderr, "Failed with error at signal block : %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}

			/*start crticial section */
			mae = MAE(point_arr,a,b,BUFF_SIZE / 2);
			mse = MSE(point_arr,a,b,BUFF_SIZE / 2);
			rmse = RMSE(point_arr,a,b,BUFF_SIZE / 2);
			free(point_arr);
			/** end critical section */

			/* UNBLOCK */
			if(sigprocmask(SIG_UNBLOCK,&block_mask,NULL) == -1){
				fprintf(stderr, "Failed with error at signal block : %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			
			all_maes[cur_index] = mae;
			all_mses[cur_index] = mse;
			all_rmses[cur_index] = rmse;
			++cur_index;

			/* if cur_index = current_size, grow the array */
			if(cur_index == current_size){
				all_maes = (double*)realloc(all_maes,sizeof(double)*(current_size + DEFAULT_POINT_SIZE));
				all_mses = (double*)realloc(all_mses,sizeof(double)*(current_size + DEFAULT_POINT_SIZE));
				all_rmses = (double*)realloc(all_rmses,sizeof(double)*(current_size + DEFAULT_POINT_SIZE));
				current_size = current_size + DEFAULT_POINT_SIZE;
			}

			sprintf(mae_str,"%.3f",mae);
			sprintf(mse_str,"%.3f",mse);
			sprintf(rmse_str,"%.3f",rmse);

			/* create the result string to be written into outputPath */
			char * res_line = (char*)calloc(strlen(line) + 50, sizeof(char));
			strcpy(res_line,line);
			strcat(res_line,",");
			strcat(res_line,mae_str);
			strcat(res_line,",");
			strcat(res_line,mse_str);
			strcat(res_line,",");
			strcat(res_line,rmse_str);
			strcat(res_line,"\n");

			/* write to output file */
			write(outpud_fd,res_line,strlen(res_line));

			/* update the input file */
			update_file(input_fd,inputPath);

			/* free allocated memory */
			free(line);
			free(res_line);
		}
		
		/*** UNLOCK **/
		lock.l_type = F_UNLCK;
		fcntl(input_fd,F_SETLKW,&lock);

		if(line_size == 0){
		
			/* if line size is 0 and p1 is finished, terminate the program */
			if(process1_finished){
				terminate_program(all_maes,all_mses,all_rmses,cur_index);
				break;
			}

			/* suspend until p1 write to temp file 
			* p1 stil runs, so suspend.
			*/
			sigset_t mask;
			sigemptyset(&mask);	
			sigsuspend(&mask);
		}
	}

	return 0;
}

void update_file(int read_fd,char * inputPath)
{
	/* read temp file into array */
	off_t size = lseek(read_fd,0,SEEK_END);
	lseek(read_fd,0,SEEK_SET);

	char * temp_file = (char*)calloc(size + 1, sizeof(char));
	
	char * delet_addr = temp_file;
	read(read_fd,temp_file,size);
	temp_file[size] = '\0';

	read_fd = open(inputPath,O_WRONLY | O_TRUNC);
	lseek(read_fd,0,SEEK_SET);
	
	int i = 0;
	while(i<size)
	{
		if(temp_file[i] == '\n')
			break;
		++i;
	}

	temp_file = &temp_file[i+1];

	write(read_fd,temp_file,strlen(temp_file));
	
	free(delet_addr);
	close(read_fd);
}

void init_signals()
{

	/** If SIGUSR1 signals comes to this process, 
	it means that, p1 finished his job.. */
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler = &handler1;
	sigaction(SIGUSR1,&sa,NULL);

	/* p1 sends the p2 to wake up */
	struct sigaction sa_2;
	memset(&sa_2,0,sizeof(sa_2));
	sa_2.sa_handler = &handler2;
	sigaction(SIGUSR2,&sa_2,NULL);

	struct sigaction sa_3;
	memset(&sa_3,0,sizeof(sa_3));
	sa_3.sa_handler = &handler_TERM;
	sigaction(SIGTERM,&sa_3,NULL);

	struct sigaction sa_4;
	memset(&sa_4,0,sizeof(sa_4));
	sa_4.sa_handler = &handler_INT;
	sigaction(SIGINT,&sa_4,NULL);
}
void terminate_program(double * all_maes,double * all_mses,double * all_rmses,int point_size)
{	
	double mean_mae = calc_mean(all_maes,point_size);
	double mean_mse = calc_mean(all_mses,point_size);
	double mean_rmse = calc_mean(all_rmses,point_size);

	double stddev_mae = calc_stddev(all_maes,mean_mae,point_size);
	double stddev_mse = calc_stddev(all_mses,mean_mse,point_size);
	double stddev_rmse = calc_stddev(all_rmses,mean_rmse,point_size);

	printf(">> [p2] process finished succesfully..\n");
	printf(">> [p2] (MAE) mean : %.3f, std dev : %.3f\n", mean_mae,stddev_mae);
	printf(">> [p2] (MSE) mean : %.3f, std dev : %.3f\n", mean_mse,stddev_mse);
	printf(">> [p2] (RMSE) mean : %.3f, std dev : %.3f\n", mean_rmse,stddev_rmse);

	cleanup();

	exit(EXIT_SUCCESS);
}

double calc_mean(double * all_values, int size)
{
	double sum = 0;

	if(size == 0){
		fprintf(stderr, "Are you sure about that size : %d ?exit.\n", size);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < size; ++i)
		sum += all_values[i];
	return sum / size;
}

double calc_stddev(double * all_values,double mean,int size)
{
	double sum = 0;
	if(size == 0){
		fprintf(stderr, "Are you sure about that size : %d ?exit.\n", size);
		exit(EXIT_FAILURE);
	}

	double used_mean = mean;

	/* If mean parameter is 1, means that calculate the mean yourself. */
	if(mean == -1)
		used_mean = calc_mean(all_values,size);

	for (int i = 0; i < size; ++i)
		sum += (all_values[i] - used_mean) * (all_values[i] - used_mean);
	sum = sum / (size - 1);

	return sqrt(sum);
}
void cleanup()
{
	close(glb_input_fd);
	close(glb_outpud_fd);

	if(glb_all_maes != NULL)
		if(*glb_all_maes != NULL)		
			free(*glb_all_maes);
	if(glb_all_mses != NULL)
		if(*glb_all_mses != NULL)
			free(*glb_all_mses);
	if(glb_all_rmses != NULL)
		if(*glb_all_rmses != NULL)
			free(*glb_all_rmses);

	unlink(glb_temp_file);
}

