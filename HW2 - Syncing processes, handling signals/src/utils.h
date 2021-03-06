/**
* CSE344 SYSTEMS PROGRAMMING
* HOMEWORK 2
* GOKTUG ALI AKIN | 161044018
*
* THIS FILE CONTAINS UTILITY FUNCTIONS
* for manipulating the point_2D
* for manipulating the arrays.
*/


#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>


/* maximum size of the string that represents the complex number */
#define TRUE 1

/* SHOULD BE (num of points * 2) */
#define BUFF_SIZE 20

/* MAX LIMIT OF FILENAME SIZE*/
#define MAX_FILE_SIZE 30

/* MAX STRING SIZE OF LSM RESULT, (ax+b) */
#define MAX_LSM_STR_SIZE 50

/* MAX STRING SIZE OF POINT , (x,y) */
#define POINT_TO_STR_SIZE 50

typedef struct 
{
	int x;
	int y;

}point_2D;


/**
* parse the command line arguments
* returns 1 if there is an error, returns 0 on succes.
*/
int parse_args(char * inputPath, char * outputPath,int argc,char *argv[]);

/**
* returns 0 if all arguments are given, returns 1 on error.
*/
int test_all_args_given(int param_i_test,int param_o_test);


/**
* returns 1 if given char is digit.
*/
int is_digit(char c);


/**
* print the usage (help) message
*/
void print_usage();


/**
* convert 20 byte line to point_2D array.
* (!) allocates dynamic memory. return addres must be free.
* @param line input line that will be converted to point array.
* @param size size of the line (BUFF_SIZE)
* BUFF_SIZE / 2 gives the number of points.
*/
point_2D * line_to_point_arr(unsigned 	char * line,size_t size);


/**
* toString for point_2D
*/
char * point_to_string(point_2D p, char * str);

/**
* print 2d point.
* @param p point to be print.
*/
void print_point(point_2D p);

/**
* @param arr input point array that will be converted to string.
* @param size number of the points
* @param res_str returns the result string.
*/
void point_arr_to_line(point_2D * arr,size_t size,char * res_str);

/**
* decodes string tokenize to 2d point.
* @param str input string to be converted to point
* @param size of the string.
* @return returns the 2d point.
*/
point_2D decode_str_to_point(const char * str,int size);

/**
* (!) allocates dynamic memory. return addres must be free.
* converts line which is generated by program1, to point array.
* @param str inputs string to be converted
* @param size number of poinst that str contains
* @return returns the point_2D array.
*/
point_2D * strline_to_point_arr(char * str, size_t size,double * a, double *b);

/**
* in critical section...
* Last Square Method
*/
void LSM(point_2D * arr,size_t size, double * x, double * y);


/** ERROR CALCULATOR FUNCTIONS **/

/**
* Calculate Mean Absolute Error
* @param arr, array that contains 2D points.
* @param line_x a value of line which is in form (ax+b)
* @param line_y b value of line which is in form (ax+b)
* @param size number of points to be processed.
* @return mean absolute error.
*/
double MAE(point_2D * arr, double line_x, double line_y,size_t size);

/**
* Calculate Mean Squared Error
* @param arr, array that contains 2D points.
* @param line_x a value of line which is in form (ax+b)
* @param line_y b value of line which is in form (ax+b)
* @param size number of points to be processed.
* @return mean squared error.
*/
double MSE(point_2D * arr, double line_x, double line_y,size_t size);

/**
* Calculate Root Mean Squared Error
* @param arr, array that contains 2D points.
* @param line_x a value of line which is in form (ax+b)
* @param line_y b value of line which is in form (ax+b)
* @param size number of points to be processed.
* @return root mean squared error.
*/
double RMSE(point_2D * arr, double line_x, double line_y,size_t size);

#endif