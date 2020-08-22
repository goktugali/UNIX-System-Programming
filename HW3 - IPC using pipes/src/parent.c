/**
* CSE344 SYSTEM PROGRAMMING
* SPRING 2020
* HOMEOWRK 3
*
* GOKTUG ALI AKIN
* 161044018
* 
*
* THIS PROGRAM CALCULATES MATRIX MULTIPLCATION (C = AxB) IN DISTRIBUTED FASHION.
* AND CALCULATES THE SINGULAR VALUE DECOMPOSIITON OF RESULT MATRIX.
* IN ORDER TO DO CALCULATION IN DISTRIBUTED FASHION, THIS PROCESS FORKS 4 CHILDS
* AND DIVIDE THE INPUT MATRICES TO QUARTERS. EVERY CHILD PROCESS CALCULATE ITS OWN QUARTERS.
* FINALLY THIS (PARENT) PROCESS COLLECTS AND COMBINE THE OUTPUTS FROM CHILDS.
*
* argument (-i) : gives the parameter of input file which contains input matrix 1.
* argument (-j) : gives the parameter of input file which contains input matrix 2.
* argument (-n) : gives the parameter of size. 
*
* EXAMPLE EXECUTION ;
* ./parent -i inputfile1.txt -j inputfile2.txt -n N
*
* WARNINGS :
* (!) (2'n)*(2'n) bytes will be readed for each file. 
* (!) THERE MUST BE AT LEAST (2'n)*(2'n) BYTES IN THE INPUT FILES.
* (!) INPUT VALUE OF N AND SIZE OF THE INPUT FILES ARE NOT LIMITED ANYWAY.
*
*
*/


#include <stdio.h>
#include <fcntl.h>
#include "utils.h"
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdarg.h>

/* print result matrice or not, for debugginh */
/* #define PRINT_RESULT_MATRICE 1 */

extern int errno;

sig_atomic_t child_exit_stat;
sig_atomic_t child_die_ctr = 0;

/* global input file descriptors */
int glb_inputA_fd,glb_inputB_fd;

/* pointer to dynamically allocated arrays */
int ** glb_mat_a = NULL;
int ** glb_mat_b = NULL;

/* store pid of all childs */
pid_t all_childs[4];


/* local functions */
/**
* initialize two pipes, one for write to parent, one for write to child.
*/
int init_pipes(int* parent_read, int* parent_write,int* child_read, int* child_write);

/*
* cleanup the allocated memory.
*/
void cleanup(int args, ... );

/**
* this function gets the result matrices (quarters) from child processes.
* combines the results that comes from child process, and return result matrice.
* this function returns heap-alloc(ed) memory addres, it must be deleted explicitly.
*/

int * collect_subproc_res(int parent_read1,int parent_read2,int parent_read3,int parent_read4,int quarter_size);

/**
* reaps the un-cleard child processes
*/
void _handler_SIGCHLD(int signo)
{   
    int status;
    pid_t pid;
    while((pid = waitpid(-1, &status, WNOHANG)) > 0){
    	/* this will clear all non-reaped childs */
    	++child_die_ctr;	
    }
}

/**
* handles and interrupts the program.
* this handler send SIGINT signal to all of its children.
*/
void _handler_SIGINT(int signo)
{

	/* iterate over the all childs */
	for (int i = 0; i < 4; ++i)
	{	
		pid_t pid = waitpid(all_childs[i],NULL,WNOHANG);
		/*
		* if pid is pid, means that process was finished, and now
		* deleted from process table by this call. There is nothing to do.
		* but if pid is 0, it means that it is not died yet, so go kill it.
		*/
		if(pid == 0){
			if(kill(all_childs[i],SIGINT) == -1){
				fprintf(stderr, "Error occured at killing child.\n");
				break;
			}

			/* clear it from process table (avoid zombies) */
			pid = waitpid(all_childs[i],NULL,WNOHANG);
		}	
	}
	
	/** clear resources **/
	if(glb_mat_a != NULL  && glb_mat_b != NULL)
		cleanup(2,*glb_mat_a,*glb_mat_b);

	close(glb_inputA_fd);
	close(glb_inputB_fd);
	exit(0);
}

int main(int argc, char *argv[])
{
	
	/* setup SIGCHLD handler */
	struct sigaction sigchld_action;
	memset(&sigchld_action,0,sizeof(sigchld_action));
	sigchld_action.sa_handler = &_handler_SIGCHLD;

	/**
	* SIGCHLD MUST NOT interrupt read / write system calls.
	* otherwise, there will be a BAD failure.
	*
	* (old )So, if SIGCHLD interrupts any system call, this syscall will be restarted.
	* WITH SA_RESTART FLAG. (deprecated)
	*
	* (new) SA_RESTART is not used, manuel macro used for restarting the system calls. (utils.h)
	* NO_EINTR(stmt) while ((stmt) == -1 && errno == EINTR)
	*
	*/
	/* sigchld_action.sa_flags = SA_RESTART; (deprecated) */
	sigaddset(&sigchld_action.sa_mask,SIGINT);
	sigaction(SIGCHLD,&sigchld_action,NULL);

	struct sigaction sigint_action;
	memset(&sigint_action,0,sizeof(sigint_action));
	sigint_action.sa_handler = & _handler_SIGINT;
	sigaddset(&sigint_action.sa_mask,SIGCHLD);
	sigaction(SIGINT,&sigint_action,NULL);

	char inputPathA[MAX_FILE_SIZE];
	char inputPathB[MAX_FILE_SIZE];
	int N;

	if(parse_args(inputPathA,inputPathB,&N,argc,argv))
		exit(EXIT_FAILURE);

	int inputA_fd = open(inputPathA, O_RDONLY);
	int inputB_fd = open(inputPathB,O_RDONLY);
	glb_inputA_fd = inputA_fd;
	glb_inputB_fd = inputB_fd;

	if(inputA_fd<0){
		fprintf(stderr, "Error opening file : %s\nProgram Terminated\n", inputPathA);
		exit(EXIT_FAILURE);
	}

	if(inputB_fd < 0){
		fprintf(stderr, "Error opening file : %s\nProgram Terminated\n", inputPathB);
		exit(EXIT_FAILURE);
	}


	int estimated_size = pow(2,N) * pow(2,N);

	/* read file into buffer */
	unsigned char * file_a = (unsigned char*)calloc(estimated_size,sizeof(unsigned char));
	unsigned char * file_b = (unsigned char*)calloc(estimated_size,sizeof(unsigned char));

	/* readed file size */
	int size;

	/* read (2'n * 2'n) bytes and check */

	size = read(inputA_fd,file_a,estimated_size);
	if(size < estimated_size){
		cleanup(2,(void*)file_a,(void*)file_b);
		close(inputA_fd);
		close(inputB_fd);
		fprintf(stderr, "Input file \"%s\" not contains sufficient characters.\nexit.\n", inputPathA);
		exit(EXIT_FAILURE);
	}

	size = read(inputB_fd,file_b,estimated_size);
	if(size < estimated_size){
		cleanup(2,(void*)file_a,(void*)file_b);
		close(inputA_fd);
		close(inputB_fd);
		fprintf(stderr, "Input file \"%s\" not contains sufficient characters.\nexit.\n", inputPathB);
		exit(EXIT_FAILURE);
	}

	/* ----------------------------------------- */
	/* continue to execution of parent */


	/* convert string, from file to int array */
	int * mat_a = string_to_matrice(file_a,estimated_size);
	int * mat_b = string_to_matrice(file_b,estimated_size);

	glb_mat_a = &mat_a;
	glb_mat_b = &mat_b;

	sigset_t set2;
	sigfillset(&set2);

	/** should not be interrupted ! critical **/
	sigprocmask(SIG_BLOCK,&set2,NULL);

	/* close input file and clear memory */

	free(file_a);
	free(file_b);

	file_a = NULL;
	file_b = NULL;

	sigprocmask(SIG_UNBLOCK,&set2,NULL);
	/** end of critical section **/

	close(inputA_fd);
	close(inputB_fd);

	/** sqrt of total size, kenar uzunlugu */
	int quarter_size = pow(2,N);

	int * child_input1 = (int*)calloc(estimated_size / 2,sizeof(int));
	int * child_input2 = (int*)calloc(estimated_size / 2,sizeof(int));

	/* setup pipes. get pipe descriptors */
	int parent_read1,parent_write1,child_read1,child_write1;
	int errrpipe1 = init_pipes(&parent_read1,&parent_write1,&child_read1,&child_write1);

	int parent_read2,parent_write2,child_read2,child_write2;
	int errrpipe2 = init_pipes(&parent_read2,&parent_write2,&child_read2,&child_write2);

	int parent_read3,parent_write3,child_read3,child_write3;
	int errrpipe3 = init_pipes(&parent_read3,&parent_write3,&child_read3,&child_write3);

	int parent_read4,parent_write4,child_read4,child_write4;
	int errrpipe4 = init_pipes(&parent_read4,&parent_write4,&child_read4,&child_write4);

	if(errrpipe1 == -1 || errrpipe2 == -1 || errrpipe3 == -1 || errrpipe4 == -1){
		fprintf(stderr, "Error at setuping pipes : %s\nexit.\n", strerror(errno));
		cleanup(4,(void*)child_input1,(void*)child_input2,(void*)mat_a,(void*)mat_b);
		exit(EXIT_FAILURE);
	}

	/* start forking */
	/* create child 1 */
	fflush(stdout);
	all_childs[0] = fork();
	if(all_childs[0] == 0){
			
		close(parent_read1);
		close(parent_write1);

		/* file descriptors as string */
		char cw[10];
		char cr[10];

		/* size az string */
		char size[10];

		sprintf(cw,"%d",child_write1);
		sprintf(cr,"%d",child_read1);
		sprintf(size,"%d",N);

		char *argvs[] = {"child",size,cw,cr,(char*)0};
		execve(argvs[0],argvs,NULL);

	}
	else if(all_childs[0] < 0){
		fprintf(stderr, "Error occured when forking : %s\n",strerror(errno));
		cleanup(4,(void*)child_input1,(void*)child_input2,(void*)mat_a,(void*)mat_b);
		exit(EXIT_FAILURE);
	}

	/* create child 2 */
	fflush(stdout);
	all_childs[1] = fork();
	if(all_childs[1] == 0){

		close(parent_read2);
		close(parent_write2);

		/* file descriptors as string */
		char cw[10];
		char cr[10];
		/* size az string */
		char size[10];

		sprintf(cw,"%d",child_write2);
		sprintf(cr,"%d",child_read2);
		sprintf(size,"%d",N);

		char *argvs[] = {"child",size,cw,cr,(char*)0};
		execve(argvs[0],argvs,NULL);

	}
	else if(all_childs[1] < 0){
		fprintf(stderr, "Error occured when forking : %s\n",strerror(errno));
		cleanup(4,(void*)child_input1,(void*)child_input2,(void*)mat_a,(void*)mat_b);
		exit(EXIT_FAILURE);
	}

	/* create child 3 */
	fflush(stdout);
	all_childs[2] = fork();
	if(all_childs[2] == 0){
		
		close(parent_read3);
		close(parent_write3);

		/* file descriptors as string */
		char cw[10];
		char cr[10];

		/* size az string */
		char size[10];

		sprintf(cw,"%d",child_write3);
		sprintf(cr,"%d",child_read3);
		sprintf(size,"%d",N);

		char *argvs[] = {"child",size,cw,cr,(char*)0};
		execve(argvs[0],argvs,NULL);
	}
	else if(all_childs[2] < 0){
		fprintf(stderr, "Error occured when forking : %s\n",strerror(errno));
		cleanup(4,(void*)child_input1,(void*)child_input2,(void*)mat_a,(void*)mat_b);
		exit(EXIT_FAILURE);
	}

	/* create child 4 */
	fflush(stdout);
	all_childs[3] = fork();
	if(all_childs[3] == 0){
		
		close(parent_read4);
		close(parent_write4);

		/* file descriptors as string */
		char cw[10];
		char cr[10];
		/* size az string */
		char size[10];

		sprintf(cw,"%d",child_write4);
		sprintf(cr,"%d",child_read4);
		sprintf(size,"%d",N);

		char *argvs[] = {"child",size,cw,cr,(char*)0};
		execve(argvs[0],argvs,NULL);
	}
	else if(all_childs[3] < 0){
		fprintf(stderr, "Error occured when forking : %s\n",strerror(errno));
		cleanup(4,(void*)child_input1,(void*)child_input2,(void*)mat_a,(void*)mat_b);
		exit(EXIT_FAILURE);;
	}

	close(child_read1);
	close(child_read2);
	close(child_read3);
	close(child_read4);
	close(child_write1);
	close(child_write2);
	close(child_write3);
	close(child_write4);

	/* child creation finish */

	/* send sub matrices to (quartes) to childs */

	/* send to child 1*/

	get_child1_inputs(mat_a,mat_b,child_input1,child_input2,quarter_size);	
	int write_err1 = write(parent_write1,child_input1,sizeof(int) * (estimated_size / 2));
	int write_err2 = write(parent_write1,child_input2,sizeof(int) * (estimated_size / 2));
	close(parent_write1);

	/* send to child 2*/
	get_child2_inputs(mat_a,mat_b,child_input1,child_input2,quarter_size);
	int write_err3 = write(parent_write2,child_input1,sizeof(int) * (estimated_size / 2));
	int write_err4 = write(parent_write2,child_input2,sizeof(int) * (estimated_size / 2));
	close(parent_write2);

	/* send to child 3*/
	get_child3_inputs(mat_a,mat_b,child_input1,child_input2,quarter_size);
	int write_err5 = write(parent_write3,child_input1,sizeof(int) * (estimated_size / 2));
	int write_err6 = write(parent_write3,child_input2,sizeof(int) * (estimated_size / 2));
	close(parent_write3);

	/* send to child 4*/
	get_child4_inputs(mat_a,mat_b,child_input1,child_input2,quarter_size);
	int write_err7 = write(parent_write4,child_input1,sizeof(int) * (estimated_size / 2));
	int write_err8 = write(parent_write4,child_input2,sizeof(int) * (estimated_size / 2));
	close(parent_write4);


	if(write_err1 == -1 || write_err2 == -1 || write_err3 == -1 || write_err4 == -1 
		|| write_err5 == -1 || write_err6 == -1 || write_err7 == -1 || write_err8 == -1){

		fprintf(stderr, "Error occured writing to child : %s \n", strerror(errno));
		cleanup(4,(void*)child_input1,(void*)child_input2,(void*)mat_a,(void*)mat_b);
		exit(EXIT_FAILURE);
	}

	free(child_input1);
	free(child_input2);


	/** clearing the memory is critical section, should not be 
	interrupted by any signal **/

	sigset_t set;
	sigfillset(&set);
	sigprocmask(SIG_BLOCK,&set,NULL);

	/* free input matrices */
	free(mat_a);
	free(mat_b);
	mat_a = NULL;
	mat_b = NULL;

	sigprocmask(SIG_UNBLOCK,&set,NULL);
	/** end of critical section **/

	/* get the result matrice */

	int * res_mat = collect_subproc_res(parent_read1,parent_read2,parent_read3,parent_read4,estimated_size / 4);

	#ifdef PRINT_RESULT_MATRICE
		char print_info[40];
		sprintf(print_info,"Matrice calculated : \n\n");
		NO_EINTR(write(1,print_info,strlen(print_info)));

		print_matrice(res_mat,quarter_size,quarter_size);

		for (int i = 0; i < quarter_size*5; ++i)
			NO_EINTR(write(1,"-",1));
		NO_EINTR(write(1,"\n\n",2));
	#endif

	/**
	* avoid child process to become an orphan.
	* parent must exit after their children exits.
	* SIGCHLD handleri, yakalayabildigi butun cocuklari temizleyecektir.
	* ancak, olasi sinyal kaybi durumunda, asagidaki dongu temizlenmemis cocugu temizleyecektir.
	* ve parent childlerdan once olurse, temizle islemi init'e kalacaktir.
	* bunun engellenmesi, cocuklarin parent tarafindan oldurulmesi gerekir.
	*/	

	pid_t pid;
	int status;
	while((pid = wait(&status)) > 0 )
		++child_die_ctr;

	/* continue to calculate SVD */

	float ** res_mat_2D = (float**)calloc(quarter_size,sizeof(float*));
	float ** v = (float**)calloc(quarter_size,sizeof(float*));
	float * w = (float*)calloc(quarter_size,sizeof(float));

	for (int i = 0; i < quarter_size; ++i){
		res_mat_2D[i] = (float*)calloc(quarter_size,sizeof(float));
		v[i] = (float*)calloc(quarter_size,sizeof(float));
	}
	
	convert_1D_to_2D(res_mat,res_mat_2D,quarter_size,quarter_size);
	free(res_mat);
	
	dsvd(res_mat_2D,quarter_size,quarter_size,w,v);
	
	char svd_res[30];
	sprintf(svd_res,"SVD calculated ; \n\n");
	NO_EINTR(write(1,svd_res,strlen(svd_res)));

	for (int i = 0; i < quarter_size; ++i)
	{
		char str[50];
		sprintf(str,"(%.3f),",w[i]);
		if(i == quarter_size -1){
			NO_EINTR(write(1,str,strlen(str)-1));
		}
		else
			NO_EINTR(write(1,str,strlen(str)));
	}
	NO_EINTR(write(1,"\n",1));

	/** free here **/

	free(w);
	for (int i = 0; i < quarter_size; ++i){
		free(res_mat_2D[i]);
		free(v[i]);
	}

	free(v);
	free(res_mat_2D);
	return 0;
}

int init_pipes(int* parent_read, int* parent_write,int* child_read, int* child_write)
{

	int fd[2];
	int pipeErr = pipe(fd);

	*parent_write = fd[1];
	*child_read = fd[0];

	int fd2[2];
	int pipeErr2 = pipe(fd2);

	*child_write = fd2[1];
	*parent_read = fd2[0];

	if(pipeErr == -1 || pipeErr2 == -1)
		return -1;
	return 0;
}


void cleanup(int args, ... )
{
	va_list list;
	va_start(list, args); 
	for (int i = 0; i < args; ++i)
	{
		void * addr = va_arg(list,void*);
		if(addr != NULL)
			free(addr);
	}
	va_end(list);
}
int * collect_subproc_res(int parent_read1,int parent_read2,int parent_read3,int parent_read4,int quarter_size)
{

	int* child1_res = (int*)calloc(quarter_size,sizeof(int));
	int* child2_res = (int*)calloc(quarter_size,sizeof(int));
	int* child3_res = (int*)calloc(quarter_size,sizeof(int));
	int* child4_res = (int*)calloc(quarter_size,sizeof(int));


	int cnt;
	NO_EINTR(cnt = read(parent_read1,child1_res,quarter_size*sizeof(int)));
	close(parent_read1);
	if(cnt < 0 ){
		fprintf(stderr, "Error ocured at read syscall (PR1) : %s\n", strerror(errno));
		cleanup(4,(void*)child1_res,(void*)child2_res,(void*)child3_res,(void*)child4_res);
		exit(EXIT_FAILURE);	
	}

	NO_EINTR(cnt = read(parent_read2,child2_res,quarter_size*sizeof(int)));
	close(parent_read2);
	if(cnt < 0){
		fprintf(stderr, "Error ocured at read syscall (PR1) : %s\n", strerror(errno));
		cleanup(4,(void*)child1_res,(void*)child2_res,(void*)child3_res,(void*)child4_res);
		exit(EXIT_FAILURE);
	}

	NO_EINTR(cnt = read(parent_read3,child3_res,quarter_size*sizeof(int)));
	close(parent_read3);
	if(cnt < 0){
		fprintf(stderr, "Error ocured at read syscall (PR1) : %s\n", strerror(errno));
		cleanup(4,(void*)child1_res,(void*)child2_res,(void*)child3_res,(void*)child4_res);
		exit(EXIT_FAILURE);
	}

	NO_EINTR(cnt = read(parent_read4,child4_res,quarter_size*sizeof(int)));
	close(parent_read4);
	if(cnt < 0){
		fprintf(stderr, "Error ocured at read syscall (PR1) : %s\n", strerror(errno));
		cleanup(4,(void*)child1_res,(void*)child2_res,(void*)child3_res,(void*)child4_res);
		exit(EXIT_FAILURE);
	}

	/* concatenate sub matrices to result matrice */

	int* res_mat = (int*)calloc(quarter_size*4,sizeof(int)); 

	quarter_size = sqrt(quarter_size*4);
	
	/* add child 1 */
	for (int i = 0; i < quarter_size / 2; ++i)
		for (int j = 0; j < quarter_size / 2; ++j)
			res_mat[j + i*quarter_size] = child1_res[j + i*(quarter_size/2)];

	/* add child 2 */
	int ic = 0;
	int jc = 0;
	for (int i = 0; i < quarter_size/2; ++i,ic++,jc = 0)
		for (int j = quarter_size/2; j < quarter_size; ++j,jc++)
			res_mat[j + i*quarter_size] = child2_res[jc + ic*(quarter_size/2)];
		
	/* add child 3 */
	ic = 0;
	jc = 0;
	for (int i = quarter_size/2; i < quarter_size; ++i,ic++,jc=0)
		for (int j = 0; j < quarter_size/2; ++j,jc++)
			res_mat[j + i*quarter_size] = child3_res[jc + ic*(quarter_size/2)];

	/* add child 4 */
	ic = 0;
	jc = 0;
	for (int i = quarter_size/2; i < quarter_size; ++i,ic++,jc=0)
		for (int j = quarter_size/2; j < quarter_size; ++j,jc++)
			res_mat[j + i*quarter_size] = child4_res[jc + ic*(quarter_size/2)];
		

	free(child1_res);
	free(child2_res);
	free(child3_res);
	free(child4_res);

	return res_mat;
}