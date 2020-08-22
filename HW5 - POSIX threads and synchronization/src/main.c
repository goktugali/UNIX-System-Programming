#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include "utils.h"


/********* GLOBAL VARIABLES ***********/
int total_florist;
int total_client;
florist_t* all_florists;
client_t* all_clients;

int* florist_numbers;
pthread_t* florist_threads;
pthread_mutex_t* all_mutexes;
pthread_cond_t* all_condvars;
request_queue** all_queues;

/* represents the total number of the flower delivered to clients */
int total_delivered = 0;
pthread_mutex_t total_delivered_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t total_delivered_condvar = PTHREAD_COND_INITIALIZER;
int delivery_done = 0;

/* represents the florists are done or not */
int florist_done = 0;
pthread_mutex_t florist_done_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t florist_done_condvar = PTHREAD_COND_INITIALIZER;

/* resources cleared or not */
int resources_cleared = 0;


/******* LOCAL FUNCTIONS *******/

/* Initialize the florists and clients with input file. */
void __init__(const char* filename);

/* clear allocated resources */
void __clear_memory__();

/* return the number of closes florist */
int get_closest_florist(const client_t* client);

/* returns true (1), if florist contains the specified flower */
int contains_flower(const florist_t* florist, const char* flower);

/* print sale statics for florist */
void print_sale_statics(const sale_static_t* s_static, const florist_t* florist);

/* Florist threads function */
void* run_florist(void* arg);

/* start florist threads */
void start_florists();

/* Central thread function. (not thread function) */
void central_thread();


void handler_SIGINT()
{
	if(!resources_cleared){

		/* stop alll threads */
		for (int i = 0; i < total_florist; ++i)
			pthread_cancel(florist_threads[i]);
		
		/* join, clear them */
		for (int i = 0; i < total_florist; ++i)
			pthread_join(florist_threads[i],NULL);

		/* clear allocated memory */
		__clear_memory__();
	}

	exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{	

	srand(time(NULL));

	sigset_t set_sigint_blocked;
	sigemptyset(&set_sigint_blocked);
	sigaddset(&set_sigint_blocked,SIGINT);

	/* setup the SIGINT handler */
	struct sigaction sigint_action;
	memset(&sigint_action,0,sizeof(sigint_action));
	sigint_action.sa_handler = &handler_SIGINT;
	sigaction(SIGINT,&sigint_action,NULL);

	/* parse command line arguments */
	char* filepath = (char*)calloc(sizeof(char),200);
	if(parse_args(filepath,argc,argv)){
		exit(EXIT_FAILURE);
	}

	char info_msg[250] = "Florist application initializing from file : ";
	sprintf(info_msg,"Florist application initializing from file : %s\n",filepath);
	write(STDOUT_FILENO,info_msg,strlen(info_msg));

	/**
	* WHY block SIGINT here : critical section
	* pthread_cancel is not safe for uninitialized threads. Behaviour is undefined if
	* pthread_cancel called with uninitialized and invalid threads.
	*
	* So, SIGINT signal will be blocked until all threads created and all memory is allocated.
	*/
	sigprocmask(SIG_BLOCK,&set_sigint_blocked,NULL);
	
	/* initialize necessary tools */
	__init__(filepath);
	free(filepath);

	int len_digit = (int)floor(log10(total_florist)) + 1;
	char* info_msg2 = (char*)calloc(sizeof(char),len_digit + 50);
	sprintf(info_msg2,"%d florists have ben created\n",total_florist);
	write(STDOUT_FILENO,info_msg2,strlen(info_msg2));
	free(info_msg2);

	char info_msg3[50] = "Processing requests..\n\n";
	write(STDOUT_FILENO,info_msg3,strlen(info_msg3));

	/* unblock the SIGINT, end of critical section */
	sigprocmask(SIG_UNBLOCK,&set_sigint_blocked,NULL);

	/* start florist threads */
	start_florists();

	/*
	*
	*
	*/

	/** CENTRAL THREAD STARTS HERE **/

	/* give all requests to the florists */
	central_thread();

	/** 
	* wait all deliveries are done by florists
	* synchronization barrier */

	pthread_mutex_lock(&total_delivered_mutex);
	while(total_delivered < total_client)
		pthread_cond_wait(&total_delivered_condvar,&total_delivered_mutex);
	pthread_mutex_unlock(&total_delivered_mutex);

	char info_end[50] = "All requests processed.\n";
	write(STDOUT_FILENO,info_end,strlen(info_end));

	/* let florist threads know it is time to terminate their execution */

	for (int i = 0; i < total_florist; ++i) // lock all mutexes
		pthread_mutex_lock(&all_mutexes[i]);

	delivery_done = 1; 

	for (int i = 0; i < total_florist; ++i) // wake up all florists
		pthread_cond_signal(&all_condvars[i]);

	for (int i = 0; i < total_florist; ++i)
		pthread_mutex_unlock(&all_mutexes[i]);


	/** 
	* wait all florist closing the shop to print the results. 
	* synchronization barrier. */
	pthread_mutex_lock(&florist_done_mutex);
	while(florist_done < total_florist)
		pthread_cond_wait(&florist_done_condvar,&florist_done_mutex);
	
	
	/* clear the resources and collect sale statics */

	write(STDOUT_FILENO,"\n",1);
	char msg[50] = "-------------------------------------------------";
	write(STDOUT_FILENO,msg,strlen(msg));
	write(STDOUT_FILENO,"\n",1);

	char mmsg1[20] = "Florist";
	char mmsg2[20] = "# of sales";
	char mmsg3[20] = "Total time";

	char mmsg4[200];
	sprintf(mmsg4,"%-10s %-15s %-10s\n",mmsg1,mmsg2,mmsg3);
	write(STDOUT_FILENO,mmsg4,strlen(mmsg4));
	write(STDOUT_FILENO,msg,strlen(msg));
	write(STDOUT_FILENO,"\n",1);


	/**
	* WHY block SIGINT here : 
	* pthread_cancel is not safe for already "joined threads". Behaviour is undefined if pthread cancel
	* calling on invalid & joined thread.
	* And also, clearing, de-allocating the heap memory is crucial, it must not be blocked.
	*
	* So, SIGINT signal will be blocked until all threads joined and heap memory cleared.
	*/
	sigprocmask(SIG_BLOCK,&set_sigint_blocked,NULL);

	for (int i = 0; i < total_florist; ++i)
	{	
		void* res_statics;
		pthread_join(florist_threads[i],&res_statics);

		print_sale_statics((sale_static_t*)res_statics,&all_florists[i]);
		free(res_statics);
	}
		
	__clear_memory__();	

	resources_cleared = 1;
	sigprocmask(SIG_UNBLOCK,&set_sigint_blocked,NULL);

	return 0;
}

void* run_florist(void* arg)
{
	/* set cancel type and cancel state */ 	
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);

	int florist_no = *((int*)arg);

	/* location of the related florist */
	point_t florist_point = all_florists[florist_no].pt;

	/* sale static variables */
	int sales_total = 0;
	long ms_total = 0;

	for(;;)
	{	
		/* lock the mutex for queue */
		pthread_mutex_lock(&all_mutexes[florist_no]);
		
		/* if no request, wait */
		while(all_queues[florist_no]->size == 0 && !delivery_done)
			pthread_cond_wait(&all_condvars[florist_no],&all_mutexes[florist_no]);

		if(all_queues[florist_no]->size == 0 && delivery_done)
			break;

		/* get client information from request queue */
		client_t client = queue_pop(all_queues[florist_no]);

		/** 
		* unlock the mutex, so central thread continue its job without 
		* waiting the delivery of this florist.
		*/
		pthread_mutex_unlock(&all_mutexes[florist_no]);

		/* start preparing and deliver the flower */

		/* get distance and speed */
		double distance = CHEBYSHEV(florist_point.x,florist_point.y,client.pt.x,client.pt.y);
		double speed = all_florists[florist_no].speed;

		/* required time to delivery */
		double deliver_time = distance / speed;
		
		/* get total sleep time */
		long total_sleep_time = (int)deliver_time + rand_select(250);
		sleep_ms(total_sleep_time); 

		/* print the info msg */
		int len_str = strlen(all_florists[florist_no].name) + strlen(client.requested_item) + strlen(client.name) + 40;
		char* msg = (char*)calloc(sizeof(char),len_str);
		sprintf(msg,"Florist %s has delivered a %s to %s in %ldms\n",all_florists[florist_no].name,client.requested_item,client.name,total_sleep_time);
		write(STDOUT_FILENO,msg,strlen(msg));
		free(msg);

		/* increment the delivered, delivered is done */
		pthread_mutex_lock(&total_delivered_mutex);
		++total_delivered;
		pthread_cond_signal(&total_delivered_condvar);
		pthread_mutex_unlock(&total_delivered_mutex);

		++sales_total;
		ms_total += total_sleep_time;
	}

	/* print closing message */
	int len_str = strlen(all_florists[florist_no].name) + 40;
	char* msg = (char*)calloc(sizeof(char),len_str);
	sprintf(msg,"Florist %s is closing the shop.\n",all_florists[florist_no].name);
	write(STDOUT_FILENO,msg,strlen(msg));
	free(msg);

	sale_static_t* s_statics = (sale_static_t*)calloc(sizeof(sale_static_t),1);
	s_statics->num_sales = sales_total;
	s_statics->total_ms = ms_total;

	/* increment florist done */
	pthread_mutex_lock(&florist_done_mutex);
	++florist_done;
	pthread_cond_signal(&florist_done_condvar);
	pthread_mutex_unlock(&florist_done_mutex);

	return (void*)s_statics;
}

void central_thread()
{	
	/* start push items to client's request queues */
	for (int i = 0; i < total_client; ++i)
	{	
		/* find closest florist number according to distance */
		int closest_florist = get_closest_florist(&all_clients[i]);

		/* lock the related mutex for guard the queue */
		pthread_mutex_lock(&all_mutexes[closest_florist]);

		/* push the request to the related florist's queue */
		queue_push(all_queues[closest_florist],all_clients[i]);

		/* signal the related florist */
		pthread_cond_signal(&all_condvars[closest_florist]);

		/* unlock the mutex */
		pthread_mutex_unlock(&all_mutexes[closest_florist]);
	}
}


void __init__(const char* filename)
{

	int total_line,client_start;
	char** file_contents = read_file(filename,&total_line,&client_start);

	int client_num = total_line - client_start;
	int florist_num = client_start;

	total_florist = florist_num;
	total_client = client_num;

	/* alloc for florists and clients */
	all_florists = (florist_t*)calloc(sizeof(florist_t),florist_num);
	all_clients = (client_t*)calloc(sizeof(client_t),client_num);

	/* initialize them with file contents */
	for (int i = 0; i < florist_num; ++i)
		all_florists[i] = str_to_florist(file_contents[i]);
	
	int index = 0;
	for (int i = client_start; i < total_line; ++i)
	{
		all_clients[index] = str_to_client(file_contents[i]);
		index++;
	}

	/* clear file contents */
	for (int i = 0; i < total_line; ++i)
		free(file_contents[i]);
	free(file_contents);


	/* alloc for florist threads */
	florist_threads = (pthread_t*)calloc(sizeof(pthread_t),total_florist);

	/* alloc for mutexes */
	all_mutexes = (pthread_mutex_t*)calloc(sizeof(pthread_mutex_t),total_florist);
	for (int i = 0; i < total_florist; ++i)
		pthread_mutex_init(&all_mutexes[i],NULL);
	
	all_condvars = (pthread_cond_t*)calloc(sizeof(pthread_cond_t),total_florist);
	for (int i = 0; i < total_florist; ++i)
		pthread_cond_init(&all_condvars[i],NULL);

	all_queues = (request_queue**)calloc(sizeof(request_queue*),total_florist);
	for (int i = 0; i < total_florist; ++i)
		all_queues[i] = create_queue(total_client);

	florist_numbers = (int*)calloc(sizeof(int),total_florist);
	for (int i = 0; i < total_florist; ++i)
		florist_numbers[i] = i;

}

int get_closest_florist(const client_t* client)
{

	double min_distance,temp_distance;
	int closest_florist;

	point_t pt1;
	point_t client_pt = client->pt;

	for (int i = 0; i < total_florist; ++i)
	{
		if(contains_flower(&all_florists[i],client->requested_item)){
			pt1 = all_florists[i].pt;
			closest_florist = i;
			break;
		}
	}

	min_distance = CHEBYSHEV(pt1.x,pt1.y,client_pt.x,client_pt.y);
	
	for (int i = 0; i < total_florist; ++i)
	{	
		if(contains_flower(&all_florists[i],client->requested_item))
		{	
			pt1 = all_florists[i].pt;
			temp_distance = CHEBYSHEV(pt1.x,pt1.y,client_pt.x,client_pt.y);

			if(temp_distance < min_distance){
				min_distance = temp_distance;
				closest_florist = i;
			}
		}
	}

	return closest_florist;
}

int contains_flower(const florist_t* florist, const char* flower)
{
	for (int i = 0; i < florist->num_items; ++i)
		if(strcmp(flower,florist->items[i]) == 0)	
			return 1;	
	return 0;
}

void start_florists()
{
	for (int i = 0; i < total_florist; ++i)
		pthread_create(&florist_threads[i],NULL,run_florist,&florist_numbers[i]);
}

void print_sale_statics(const sale_static_t* s_static, const florist_t* florist)
{
	int len_str = 0;
	if((int)s_static->total_ms != 0)
		len_str += (int)floor(log10(s_static->total_ms)) + 1;
	else
		len_str += 1;

	if(s_static->num_sales != 0)
		len_str += (int)floor(log10(s_static->num_sales)) + 1;
	else
		len_str += 1;

	len_str += strlen(florist->name);

	char* res_str = (char*)calloc(sizeof(char),len_str + 50);
	sprintf(res_str,"%-10s %-15d %dms\n",florist->name,s_static->num_sales,(int)s_static->total_ms);
	write(STDOUT_FILENO,res_str,strlen(res_str));
	free(res_str);
}

void __clear_memory__()
{
	for (int i = 0; i < total_florist; ++i)
		pthread_mutex_destroy(&all_mutexes[i]);
	free(all_mutexes);

	for (int i = 0; i < total_florist; ++i)
		pthread_cond_destroy(&all_condvars[i]);
	free(all_condvars);
	
	for (int i = 0; i < total_florist; ++i)
		free(all_queues[i]->items);
	
	for (int i = 0; i < total_florist; ++i)
		free(all_queues[i]);
	free(all_queues);

	for (int i = 0; i < total_florist; ++i)
	{	
		for (int j = 0; j < all_florists[i].num_items ; ++j)
			free(all_florists[i].items[j]);
		
		free(all_florists[i].items);
		free(all_florists[i].name);	
	}

	for (int i = 0; i < total_client; ++i)
	{
		free(all_clients[i].name);
		free(all_clients[i].requested_item);
	}

	free(all_florists);
	free(all_clients);
	free(florist_threads);
	free(florist_numbers);
}

