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
* THIS SOURCE CODE CONTAINS THE CHILD (WORKERS) PROCESS.
* 
* DO NOT RUN THIS FILE. THIS WILL BE EXECUTED BY PARENT.
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

extern int errno;


int ** glb_mat_1 = NULL;
int ** glb_mat_2 = NULL;

void multiplyMatrices(int* matA, int* matB,int* matC,int N);

void _handler_SIGINT(int signo)
{

	if(glb_mat_1 != NULL)
		if(*glb_mat_1 != NULL)
			free(*glb_mat_1);
	if(glb_mat_2 != NULL)
		if(*glb_mat_2 != NULL)
			free(*glb_mat_2);

	exit(0);
}

int main(int argc, char const *argv[])
{	

	struct sigaction sigint_action;
	memset(&sigint_action,0,sizeof(sigint_action));
	sigint_action.sa_handler = & _handler_SIGINT;
	sigaction(SIGINT,&sigint_action,NULL);

	
	int n = atoi(argv[1]);
	int write_fd = atoi(argv[2]);
	int read_fd = atoi(argv[3]); 

	if(n == 0 || write_fd == 0 || read_fd == 0){
		fprintf(stderr, "Invalid arguments given to child process !\nexit.\n");
		exit(EXIT_FAILURE);
	}

	int quarter_size = pow(2,n);
	int bytes_size = (quarter_size * quarter_size) / 2;

	/* read input 1 */
	int * mat1 = (int*)calloc(bytes_size,sizeof(int));
	glb_mat_1 = &mat1;
	int read_err1 = read(read_fd,mat1,bytes_size * sizeof(int));


	/* read input 1 */
	int * mat2 = (int*)calloc(bytes_size,sizeof(int));
	glb_mat_2 = &mat2;
	int read_err2 = read(read_fd,mat2,bytes_size * sizeof(int));

	/* result of multiply matrice */
	int * res = (int*)calloc((bytes_size / 2),sizeof(int));

	if(read_err1 == -1 || read_err2 == -1){
		free(mat1);
		free(mat2);
		free(res);
		close(write_fd);
		close(read_fd);

		fprintf(stderr, "Error occured at read system call : %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* calculate and write result */
	multiplyMatrices(mat1,mat2,res,quarter_size);
	write(write_fd,res,sizeof(int)*(bytes_size / 2));
	close(write_fd);
	close(read_fd);

	sigset_t set;
	sigfillset(&set);

	/* critical section */
	sigprocmask(SIG_BLOCK,&set,NULL);
	free(res);
	free(mat2);
	mat2 = NULL;
	free(mat1);
	mat1 = NULL;
	sigprocmask(SIG_UNBLOCK,&set,NULL);
	/* end of critical section */
	return 0;
}

void multiplyMatrices(int* matA, int* matB,int* matC,int N) 
{
    for (int i = 0; i < N/2; i++) {
        for (int j = 0; j < N/2; j++) {
            int sum = 0.0;
            for (int k = 0; k < N; k++)
                sum = sum + matA[i * N + k] * matB[k * N/2 + j];
            matC[i * N/2 + j] = sum;
        }
    }
}

