/**
* CSE344 SYSTEMS PROGRAMMING
* PROJECT 5
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
#include <math.h>
#include <time.h>

extern int errno;

#define NO_EINTR(stmt) while ((stmt) == -1 && errno == EINTR);

#define MAX(x,y) ((x>y) ? x:y)

#define CHEBYSHEV(x1,y1,x2,y2) (MAX(fabs(x2-x1),fabs(y2-y1)))

#define TRUE 1
#define FALSE 0


typedef struct point
{
	float x;
	float y;

}point_t;


typedef struct florist
{
	point_t pt;
	float speed;
	char** items;
	char* name;
	int num_items;
	int total_time;

}florist_t;


typedef struct client
{	
	point_t pt;
	char* name;
	char* requested_item;

}client_t;


typedef struct statics
{
	int num_sales;
	long total_ms;
	
}sale_static_t;


typedef struct queue
{
	client_t* items;
	int front;
	int rear;
	int size;
	unsigned int capacity;

}request_queue;

/* utility functions */

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
* usleep() deprecated in some POSIX systems.
* manual implementation for sleeping in miliseconds.
* source : (https://stackoverflow.com/questions/1157209/is-there-an-alternative-sleep-function-in-c-to-milliseconds)
*/
int sleep_ms(long msec);

/**
* read and parse the file into char array.
*/
char** read_file(const char* filepath,int* lines,int* client_start);

/**
* return the number of c in the given string.
*/
int char_count(const char* string, char c,size_t size);


/**
* exit under illegal file conditions.
*/
void _exit_invalid_file(const char* filepath);

/**
* Remove whitespaces from string, returns new string.
*/
char* remove_whitespace(const char* token);

/**
* Return point with given token string.
*/
point_t str_to_point(const char* token);

/**
* Return florist with given token string.
*/
florist_t str_to_florist(const char* token);

/**
* Return client with given token string.
*/
client_t str_to_client(const char* token);


/* request queue functions */

/**
* Return empty queue with given size.
*/
request_queue* create_queue(unsigned int size);

/**
* Push item to the queue
*/
void queue_push(request_queue* src_queue, client_t item);

/**
* Pop item from queue.
*/
client_t queue_pop(request_queue* src_queue);

#endif