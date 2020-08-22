/**
* CSE344 SYSTEMS PROGRAMMING
* HOMEWORK 1
* GOKTUG ALI AKIN | 161044018
*
* THIS FILE CONTAINS UTILITY FUNCTIONS
* for manipulating the complex_num
* for manipulating the arrays.
*/


#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "complex.h"


/* maximum size of the string that represents the complex number */
#define MAX_COMPLEX_STR_SIZE 30
#define MAX_FILE_SIZE 30
#define TRUE 1
#define BUFF_SIZE 32

// SAMPLING_STEPS for FFT calculation
#define SAMPLING_STEPS 1


#define IPC_FILE "ipc.txt"
#define TEMP_FILE "temp.txt"


/**
* Represent the IPC file status
*/
typedef struct ipcfilestatus
{
	int A_CNT; /* number of alive a process*/
	int B_CNT; /* number of alive b process*/
}file_stat;


/**
* check internal files (interprocess communication file)
*/
void internal_file_test();

/**
* update the ipc file status
*/
void update_file_status(file_stat status);

/**
* parse the command line arguments
* returns 1 if there is an error, returns 0 on succes.
*/
int parse_args(char * inputPath, char * outputPath, int * time,int argc,char *argv[]);

/**
* returns 0 if all arguments are given, returns 1 on error.
*/
int test_all_args_given(int param_i_test,int param_o_test,int param_t_test);


/**
* returns 1 if given char is digit.
*/
int is_digit(char c);

/**
* returns the status of file
*/
file_stat file_status();

/**
* returns 1 if given string is considered as empty.
*/
int isEmptyLine(const char * buff);

/**
* print the usage (help) message
*/
void print_usage();


/**
* Converts integer to string.
*/
void int_to_str(char * str,int number);

/**
* toString for complex number
*/
void complex_to_string(char *str,complex_num num);
	
/**
* toString for complex number, print by double
*/
void complex_to_string_double(char *str,complex_num num);	
/**
* convert ascii to complex number
*/
complex_num decode_ASCII(char str[2]);

/**
* convert string to complex number.
*/

complex_num decode_str_to_complex(const char * str,int size);

/**
* convert whole string line to complex number array. parse by ","
*/
complex_num * strline_to_complex_arr(char * str);



char * complex_arr_to_string(const complex_num* numbers,int size,char type);
complex_num* str_to_complex_arr(const char * str, int buff_size);


#endif