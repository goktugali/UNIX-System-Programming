/**
* CSE344 SYSTEMS PROGRAMMING
* HOMEWORK 3
* GOKTUG ALI AKIN | 161044018
*
* THIS FILE CONTAINS UTILITY FUNCTIONS
* for manipulating the matrices
* for manipulating the string etc.
*/


#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <errno.h>

#define MAX_FILE_SIZE 30
#define TRUE 1

#define Abs(x) ( (x)>0.0?  (x) : (-(x)) ) 
#define SIGN(u,v) ( (v)>=0.0 ? Abs(u) : -Abs(u) )
#define MAX(a,b) (((a)>(b))?(a):(b))


extern int errno;

#define NO_EINTR(stmt) while ((stmt) == -1 && errno == EINTR);
/**
* parse the command line arguments
* returns 1 if there is an error, returns 0 on succes.
*/
int parse_args(char * inputPathA, char * inputPathB, int * n,int argc,char *argv[]);

/**
* returns 0 if all arguments are given, returns 1 on error.
*/
int test_all_args_given(int param_i_test,int param_o_test,int param_t_test);

/**
* print the usage (help) message
*/
void print_usage();

/**
* convert string to 1D array
* return addres must be deleted.
*/
int * string_to_matrice(unsigned char * source_str,int size);

/**
* printd 1D array as matrice
*/
void print_matrice(int * matrice, int row, int column);

/**
* These functions creates the inputs for childs. Childs accept 2 matrices 
* for calculate the quarter matrices.
* For example, if input matrices are 4x4, childs will accept 2 matrices 
* by this format ;
*
* 1234   1 2
* 1234   1 2
*        1 2
*        1 2
*
* one 2x4 and 4x2 matrices to create 2x2 quarter part of result matrice.  
*/
void get_child1_inputs(const int * mat_a , const int * mat_b, int * res_input1, 
						int * res_input2, int quarter_size);
void get_child2_inputs(const int * mat_a , const int * mat_b, int * res_input1, 
						int * res_input2, int quarter_size);
void get_child3_inputs(const int * mat_a , const int * mat_b, int * res_input1, 
						int * res_input2, int quarter_size);
void get_child4_inputs(const int * mat_a , const int * mat_b, int * res_input1, 
						int * res_input2, int quarter_size);



/**
* convert 1D array to 2D array (matrice)
*/
void convert_1D_to_2D(const int * source_arr, float ** res_arr,int row,int column);


/**
* singular value calculation
* source = ()
*/
int dsvd(float **a, int m, int n, float *w, float **v);


#endif