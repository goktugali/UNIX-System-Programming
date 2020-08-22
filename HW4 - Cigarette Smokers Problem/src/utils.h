/**
* CSE344 SYSTEMS PROGRAMMING
* PROJECT 4
* GOKTUG ALI AKIN | 161044018
*
* THIS FILE CONTAINS UTILITY FUNCTIONS
*/

#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h> 
#include <signal.h>


extern int errno;

#define NO_EINTR(stmt) while ((stmt) == -1 && errno == EINTR);

#define TRUE 1
#define FALSE 0

/**
* parse the command line arguments
* returns 1 if there is an error, returns 0 on succes.
*/
int parse_args(char* filepath,int argc,char *argv[]);

/**
* returns 0 if all arguments are given, returns 1 on error.
*/
int test_all_args_given(int param_f_test);

/**
* print the usage (help) message
*/
void print_usage();

/**
* return random number between 1 to bound
*/
int rand_select(int bound);

/**
* read and parse the file into char array.
*/
char* read_file(const char* filepath);

/**
* return the number of c in the given string.
*/
int char_count(const char* string, char c,size_t size);

/**
* check that given pair (WS,MF etc) is valid or not.
*/
int is_valid_pair(char pair[2]);

/**
* exit under illegal file conditions.
*/
void _exit_invalid_file(const char* filepath);
#endif