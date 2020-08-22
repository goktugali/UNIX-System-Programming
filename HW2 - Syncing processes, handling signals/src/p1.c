#include <stdio.h>
#include "utils.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

/* global variables */
extern int errno;
sig_atomic_t p2_started = 0;
int glb_input_fd;
sigset_t pending_mask;


/* local functions */
void print_pending_sigset();
/*
* print end of program information.
*/
void print_eop_info(int read_ctr);
/*
* initialize signal handlers
*/
void init_signals();

/* handler functions */
void handler1(int signo){
	p2_started = 1;
}
void handler2(int signo){
}

void handler_TERM(int signo){

	printf(">> [p1] interrupted by SIGTERM\n");
	close(glb_input_fd);
	print_pending_sigset();
	exit(EXIT_FAILURE);
}

void handler_INT(int signo){

	printf(">> [p1] interrupted by SIGINT\n");
	close(glb_input_fd);
	print_pending_sigset();
	exit(EXIT_FAILURE);
}
/* handler functions */

int main(int argc, char *argv[])
{

	/* initialize signals */
	init_signals();

	/* If p1 was stuck, wake up by sigusr 2 
	* this sigusr2 will listen just in the end of the main
	suspend block, otherwise signal can be lost.
	*/
	sigset_t init_mask;
	if(sigemptyset(&init_mask) == -1){
		fprintf(stderr, "Signal mask cannot be initialized now\nExit.\n");
		exit(EXIT_FAILURE);
	}
	if(sigaddset(&init_mask,SIGUSR2)){
		fprintf(stderr, "Signal mask cannot be initialized now\nExit.\n");
		exit(EXIT_FAILURE);
	}
	if(sigprocmask(SIG_BLOCK,&init_mask,NULL)){
		fprintf(stderr, "Failed with error at signal block : %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	char inputPath[MAX_FILE_SIZE];
	char outputPath[MAX_FILE_SIZE];

	printf(">> [p1] process started..\n");

	/* create TEMP file */
	char temp_file[MAX_FILE_SIZE] = "tempXXXXXX";
	int temp_fd = mkstemp(temp_file);

	if(temp_fd < 0){
		fprintf(stderr, "Failed to create temp file %s\n", temp_file);
		fprintf(stderr, "Failed with error %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if(parse_args(inputPath,outputPath,argc,argv))
		exit(EXIT_FAILURE);

	int input_fd = open(inputPath, O_RDONLY);
	int output_fd = open(outputPath,O_RDONLY);

	if(input_fd<0){
		fprintf(stderr, "Error opening file : %s\nProgram Terminated\n", inputPath);
		exit(EXIT_FAILURE);
	}

	if(output_fd < 0){
		fprintf(stderr, "Error opening file : %s\nProgram Terminated\n", outputPath);
		exit(EXIT_FAILURE);
	}
	close(output_fd);
	glb_input_fd = input_fd;

	/**
	* start p2 process.
	* fork and execve(p2)
	*/

	fflush(stdout);
	pid_t pid = fork();
	if(pid == -1){
		fprintf(stderr, "Error occured when forking...\n");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)
	{	
		fflush(stdout);
		/* in child process, execute the p2 */
		char *argvs[] = {"p2","-i",temp_file,"-o",outputPath,(char*)0};
		execve(argvs[0],argvs,NULL);
	}

	/* [0-255] */
	unsigned char * buff = (unsigned char*)calloc(BUFF_SIZE+1,sizeof(unsigned char));

	/** init flock **/
	struct flock lock;
	memset(&lock,0,sizeof(lock));

	point_2D * point_arr;
	char res_str[200];
	int read_size = 0;

	sigset_t block_mask;
	if(sigemptyset(&block_mask)){
		fprintf(stderr, "Failed with error at signal mask initalize : %s\n", strerror(errno));
		close(input_fd);
		exit(EXIT_FAILURE);	
	}

	while(read(input_fd,buff,BUFF_SIZE) == BUFF_SIZE)
	{

		point_arr = line_to_point_arr(buff,BUFF_SIZE);
		point_arr_to_line(point_arr,BUFF_SIZE / 2,res_str);

		char LSM_res[MAX_LSM_STR_SIZE];
		double x,y;

		if(sigaddset(&block_mask,SIGINT) == -1 || sigaddset(&block_mask,SIGSTOP) == -1 || sigaddset(&block_mask,SIGUSR1) == -1){
			fprintf(stderr, "Failed with error at signal mask initalize : %s\n", strerror(errno));
			close(input_fd);
			exit(EXIT_FAILURE);
		}

		/* BLOCK SIGINT and SIGSTOP */
		if(sigprocmask(SIG_BLOCK,&block_mask,NULL) == -1){
			fprintf(stderr, "Failed with error at signal blocking.: %s\n", strerror(errno));
			close(input_fd);
			exit(EXIT_FAILURE);
		}

		/** critical section begins **/
		LSM(point_arr,BUFF_SIZE / 2,&x,&y); /*calculate lsm*/
		free(point_arr); 
		/** critical section ends **/

		if(sigpending(&pending_mask) == -1){
			fprintf(stderr, "Failed with error at get signal pending : %s\n", strerror(errno));
			close(input_fd);
			exit(EXIT_FAILURE);
		}

		/* UNBLOCK */
		if(sigprocmask(SIG_UNBLOCK,&block_mask,NULL) == -1){
			fprintf(stderr, "Failed with error at signal unblocking.: %s\n", strerror(errno));
			close(input_fd);
			exit(EXIT_FAILURE);
		}

		sprintf(LSM_res,"%.3fx+%.3f",x,y);
		strcat(res_str,",");
		strcat(res_str,LSM_res);

		/* now, result line is ready to write */
		/* find available line */

		/* lock the temp file */
		lock.l_type = F_WRLCK;
		fcntl(temp_fd,F_SETLKW,&lock);

		lseek(temp_fd,0,SEEK_END);
		
		write(temp_fd,res_str,strlen(res_str));
		write(temp_fd,"\n",1);

		/* unlock the temp file */
		lock.l_type = F_UNLCK;
		fcntl(temp_fd,F_SETLKW,&lock);

		/* notify the p2 (child) that, now temp file is not empty, p2 can use it */
		/* this signal wakes up the p2 program (child) */

		if(p2_started)
			kill(pid,SIGUSR2);
		++read_size;
	}

	/* clear the array for reading the input file*/
	free(buff);


	/* send the finish information to the p2 */
	/**
	* this signal must be send to p2 after p2 starts.
	* after p2 starts, p1 can send the information to p2.
	* this guard added because, if p1 sends this signal before the p2 starts,
	* it will crash.
	*/
	close(input_fd);
	print_eop_info(read_size);
	if(!p2_started){

		/* unblock SIGUSR2 and start suspend 
		* what if, p2 sends start signal(SIGUSR1) just here ?
		* solution : p2 sends another signal SIGUSR2. SIGUSR2 blocked
		* whole program except this sigsuspend state. So, suspend will 
		* be wake up when just SIGUSR2 comes.
		* wait until p2 start. 
		*/
		sigset_t set;
		sigemptyset(&set);
		sigsuspend(&set);
	}
	/* send ( I am done ) to child (p2) */
	if(kill(pid,SIGUSR1) == -1){
		fprintf(stderr, "Failed with error at signal sending : %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	/* wait p2(child) to finish
	* Parent process should die after child dies.
	* This wait call can be removed, not necessary.
	* But, there will be bad output on the terminal, since parent can die
	* before die child.
	*/
	
	waitpid(pid,NULL,0);

	return 0;
}


void print_pending_sigset()
{	
	int t_sigstop=0,t_sigint=0,t_sigusr1=0,t_sigusr2=0;
	printf(">> [p1] Signals in critical section (1 means caught):\n");

	if(sigismember(&pending_mask,SIGINT))
		t_sigint = 1;
	if(sigismember(&pending_mask,SIGSTOP))
		t_sigstop = 1;
	if(sigismember(&pending_mask,SIGUSR1))
		t_sigusr1 = 1;	
	if(sigismember(&pending_mask,SIGUSR2))
		t_sigusr2 = 1;

	printf(">> [p1] SIGUSR1 : %d\n", t_sigusr1);
	printf(">> [p1] SIGUSR2 : %d\n", t_sigusr2);
	printf(">> [p1] SIGSTOP : %d\n", t_sigstop);
	printf(">> [p1] SIGINT  : %d\n", t_sigint);

}


void print_eop_info(int read_ctr)
{	
	printf(">> [p1] process finished succesfully..\n");
	printf(">> [p1] %d total bytes readed\n", read_ctr * BUFF_SIZE);
	printf(">> [p1] %d line equations processed.\n", read_ctr);
	print_pending_sigset();
}

void init_signals()
{	
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler = &handler1;
	sigaction(SIGUSR1,&sa,NULL);

	struct sigaction sa2;
	memset(&sa2,0,sizeof(sa2));
	sa2.sa_handler = &handler2;
	sigaction(SIGUSR2,&sa2,NULL);

	struct sigaction sa_3;
	memset(&sa_3,0,sizeof(sa_3));
	sa_3.sa_handler = &handler_TERM;
	sigaction(SIGTERM,&sa_3,NULL);

	struct sigaction sa_4;
	memset(&sa_4,0,sizeof(sa_4));
	sa_4.sa_handler = &handler_INT;
	sigaction(SIGINT,&sa_4,NULL);
}