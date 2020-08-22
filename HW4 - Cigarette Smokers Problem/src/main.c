/**
* CSE344 SYSTEM PROGRAMMING
* PROJECT 4 | CIGARETTE SMOKERS derivative synchronization problem
*
* --------------------------------------------------------------------------------------------------
* Let’s assume for the sake of simplicity that one needs exactly 4 ingredients for preparing desert:
* - milk (M)
* - flour (F)
* - walnuts (W)
* - sugar (S).
* There are 6 chefs in a street, and each has an endless supply of two distinct 
* ingredients and lacks the remaining two:
* e.g.
* chef1 has an endless supply of milk and flour but lacks walnuts and sugar,
* chef2 has an endless supply of milk and sugar but lacks flour and walnuts,
* chef3 has an endless supply of milk and walnuts but lacks sugar and flour,
* etc.
* --------------------------------------------------------------------------------------------------
*
* example execution ; 
* $:./main -i filepath
*
* filepath contains the items that wholesaler will be posted.
*/

#include <stdio.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "semun.h"
#include "utils.h"
#include <time.h>

extern int errno;

#define NUM_OF_CHEFS 6
#define DESERT "GULLAC"


/**
* ID of the System V semaphore set.
* This set contains 5 semaphores.
* ---------------------------------
* sem no : 0 1 2 3        4
* meaning: M F W S desert_preapared 
* ---------------------------------
*/
int all_sems;

/**
* Store all pthread_objects.
*/
pthread_t chefs[NUM_OF_CHEFS];

/**
* Store the no of the chefs as global.
* This array items will be passed to thread as function argument.
* This should not be local, it can be out of scope when thread uses it,
* so declared as global.
*/
int chef_nums[NUM_OF_CHEFS];

/**
* shared data structure between chef threads
* wholesaler deliver items to this data structure.
*/
char* items_shared;


/******** System functions to setup necessary tools *********/

/**
* Initialize SIGINT handler.
*/
void _init_sigint();

/**
* Setups and initializes the System V semaphores by using 
* all_sems global variable.
*/
void _init_semaphores();

/**
* Clears System V semaphores.
*/
void _clear_semaphores();

/**
* Starts chef threads.
*/
void _start_chef_threads();

/**
* Clears, reapes the chef threads.
*/
void _join_chef_threads();


/**
* Cancel chef threads.
*/
void _cancel_chefs();

/*********** chefs functions ************/

/**
* Chef threads function.
* Integer parameter passed to represent the chef number.
* Return value is not necessary. (not used)
*/
void* run_chef(void* arg);


/**
* Print chef at waiting message.
* @param chefno Number of chef.
* @param item1,item2 Represent the items (milk,wulnat,sugar,flour).
*/
void print_chef_atwait(int chefno,const char* item1,const char* item2);

/**
* Print chef when he/she has taken the items.
* @param chefno Number of chef.
* @param item1,item2 Represent the items (milk,wulnat,sugar,flour).
*/
void print_chef_hastaken(int chefno,const char* item1,const char* item2);

/**
* Prepare the derst in sleeptime seconds.
* @param chefnı Number of chef.
* @param sleeptime sleep time in seconds.
*/
void prepare_desert(int chefno,int sleeptime);

/**
* Print chef when he/she delivered the desert.
* @param chefno Number of chef.
*/
void print_chef_delivered(int chefno);

/**
* Posts the semaphore (number 4 in the all_sems set) to wake up the wholesaler.
*/
void post_desert_prepared();

/**
* pop,get (delete) ingredients from shared data structure.
*/
void get_item();


/************ wholesaler functions ************/

/**
* Read items from files post these items to chefs.
* @param filepath File path to be readed.
*/
void run_wholesaler(const char* filepath);

/**
* Prints the wholesaler delivered the items message.
* @param item1,item2 represent the item names.
*/
void print_whslr_deliver(const char* item1,const char* item2);

/**
* Prints the wholesaler at waiting desert message.
*/
void print_whslr_atwait();

/**
* Prints the wholesaler whe he/she obtains the desert.
*/
void print_whslr_obtain();

/**
* Post the milk and flour semaphores in the System V semaphore set (all_sems)
*/
void post_MF();

/**
* Post the milk and walnut semaphores in the System V semaphore set (all_sems)
*/
void post_MW();

/**
* Post the milk and sugar semaphores in the System V semaphore set (all_sems)
*/
void post_MS();

/**
* Post the walnut and sugar semaphores in the System V semaphore set (all_sems)
*/
void post_WS();

/**
* Post the walnut and flour semaphores in the System V semaphore set (all_sems)
*/
void post_WF();

/**
* Post the flour and sugar semaphores in the System V semaphore set (all_sems)
*/
void post_FS();

/**
* Wait untill desert prepared by any chefs.
* number 4 in the System V semaphore set (all_sems)
*/
void wait_desert_prepared();

/**
* push two items into shared data structure.
* e.g : if item_pair == MF then push M and F.
*/
void add_item(const char* item_pair);


/**
* SIGINT handler
*/
void handler_SIGINT(int signo)
{
	_cancel_chefs();
	_join_chef_threads();
	_clear_semaphores();
	exit(EXIT_FAILURE);
}

/**
* Main thread will setup the necessary tools (System V semaphores)
* and run the wholesaler code.
*/
int main(int argc, char *argv[])
{
	
	/* init SIGINT handler */
	_init_sigint();

	char filepath[100];
	if(argc != 3){
		print_usage();
		exit(EXIT_FAILURE);
	}
	if(parse_args(filepath,argc,argv)){
		exit(EXIT_FAILURE);
	}

	/* check validity before start program */
	char * __validtest__ = read_file(filepath);
	if(strlen(__validtest__) < 20){
		fprintf(stderr, "File must contains at least 10 rows\n");
		free(__validtest__);
		exit(EXIT_FAILURE);
	}
	free(__validtest__);

	/* for random sleep time */
	srand(time(NULL));	

	/* shared array between threads */
	items_shared = (char*)calloc(sizeof(char),2);

	/** 
	* initialize System V semaphores into global all_sems variable that represents the
	* ID of the semaphore set.
	*/
	_init_semaphores();

	_start_chef_threads();

	run_wholesaler(filepath);

	/** 
	* after wholesaler is done, chefs must stop their execution.
	* main thread cancels these chefs threads.
	*/
	_cancel_chefs();
	
	/* clear threads */
	_join_chef_threads();

	/* clear semaphores */
	_clear_semaphores();

	free(items_shared);

	return 0;
}

void add_item(const char* item_pair)
{
	strcpy(items_shared,item_pair);
}
/**
* Wholesaler utility functions.
*/
void wait_desert_prepared()
{
	struct sembuf sops[1];

	sops[0].sem_num = 4; // prepared_desert semaphore
	sops[0].sem_op = -1; // wait, subtract 1
	sops[0].sem_flg = 0;

	if(-1 == semop(all_sems,sops,1)){
		char errmsg[300];
		sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
		write(STDERR_FILENO,errmsg,strlen(errmsg));
		exit(EXIT_FAILURE);
	}
}
void run_wholesaler(const char* filepath)
{	
	char* whslr_items = read_file(filepath);
	char* addr_backup = whslr_items;

	size_t size = strlen(whslr_items);
	
	char pair[3];
	
	for (int i = 0; i < size / 2; ++i)
	{
		strncpy(pair,whslr_items,2);
		pair[2] = '\0';

		// push item to shared data structure.
		add_item(pair);

		if(strcmp(pair,"MF") == 0 || strcmp(pair,"FM") == 0){
			post_MF();
			print_whslr_deliver("milk","flour");
		}
		else if(strcmp(pair,"MW") == 0 || strcmp(pair,"WM") == 0){
			post_MW();
			print_whslr_deliver("milk","walnut");
		}
		else if(strcmp(pair,"MS") == 0 || strcmp(pair,"SM") == 0){
			post_MS();
			print_whslr_deliver("milk","sugar");
		}
		else if(strcmp(pair,"FS") == 0 || strcmp(pair,"SF") == 0){
			post_FS();
			print_whslr_deliver("flour","sugar");
		}
		else if(strcmp(pair,"FW") == 0 || strcmp(pair,"WF") == 0){
			post_WF();
			print_whslr_deliver("flour","walnut");
		}
		else if(strcmp(pair,"SW") == 0 || strcmp(pair,"WS") == 0){
			post_WS();
			print_whslr_deliver("sugar","walnut");
		}
		
		// switch to next pair in the file
		whslr_items = &whslr_items[2];

		// wait until desert prepared by any chef.
		print_whslr_atwait();
		wait_desert_prepared();
		print_whslr_obtain();
	}

	free(addr_backup);

}
void post_MF()
{
	struct sembuf sops[2];

	// post M
	sops[0].sem_num = 0;
	sops[0].sem_op = 1; 
	sops[0].sem_flg = 0;

	// post F
	sops[1].sem_num = 1;
	sops[1].sem_op = 1; 
	sops[1].sem_flg = 0;

	if(-1 == semop(all_sems,sops,2)){
		char errmsg[300];
		sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
		write(STDERR_FILENO,errmsg,strlen(errmsg));
		exit(EXIT_FAILURE);
	}
}
void post_MW()
{
	struct sembuf sops[2];

	// post M
	sops[0].sem_num = 0;
	sops[0].sem_op = 1; 
	sops[0].sem_flg = 0;

	// post W
	sops[1].sem_num = 2;
	sops[1].sem_op = 1; 
	sops[1].sem_flg = 0;

	if(-1 == semop(all_sems,sops,2)){
		char errmsg[300];
		sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
		write(STDERR_FILENO,errmsg,strlen(errmsg));
		exit(EXIT_FAILURE);
	}
}
void post_MS()
{
	struct sembuf sops[2];

	// post M
	sops[0].sem_num = 0;
	sops[0].sem_op = 1; 
	sops[0].sem_flg = 0;

	// post S
	sops[1].sem_num = 3;
	sops[1].sem_op = 1; 
	sops[1].sem_flg = 0;

	if(-1 == semop(all_sems,sops,2)){
		char errmsg[300];
		sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
		write(STDERR_FILENO,errmsg,strlen(errmsg));
		exit(EXIT_FAILURE);
	}
}
void post_WS()
{
	struct sembuf sops[2];

	// post W
	sops[0].sem_num = 2;
	sops[0].sem_op = 1; 
	sops[0].sem_flg = 0;

	// post S
	sops[1].sem_num = 3;
	sops[1].sem_op = 1; 
	sops[1].sem_flg = 0;

	if(-1 == semop(all_sems,sops,2)){
		char errmsg[300];
		sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
		write(STDERR_FILENO,errmsg,strlen(errmsg));
		exit(EXIT_FAILURE);
	}
}
void post_WF()
{
	struct sembuf sops[2];

	// post F
	sops[0].sem_num = 1;
	sops[0].sem_op = 1; 
	sops[0].sem_flg = 0;

	// post W
	sops[1].sem_num = 2;
	sops[1].sem_op = 1; 
	sops[1].sem_flg = 0;

	if(-1 == semop(all_sems,sops,2)){
		char errmsg[300];
		sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
		write(STDERR_FILENO,errmsg,strlen(errmsg));
		exit(EXIT_FAILURE);
	}
}
void post_FS()
{
	struct sembuf sops[2];

	// wait (get) F
	sops[0].sem_num = 1;
	sops[0].sem_op = 1; 
	sops[0].sem_flg = 0;

	// wait (get) S
	sops[1].sem_num = 3;
	sops[1].sem_op = 1; 
	sops[1].sem_flg = 0;

	if(-1 == semop(all_sems,sops,2)){
		char errmsg[300];
		sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
		write(STDERR_FILENO,errmsg,strlen(errmsg));
		exit(EXIT_FAILURE);
	}
}
void print_whslr_obtain()
{
	char outmsg[300];
	sprintf(outmsg,"The wholesaler has obtained the %s and left to sell it\n",DESERT);
	write(STDOUT_FILENO,outmsg,strlen(outmsg));
}
void print_whslr_atwait()
{
	char outmsg[300];
	sprintf(outmsg,"The wholesaler is waiting for the %s\n",DESERT);
	write(STDOUT_FILENO,outmsg,strlen(outmsg));
}
void print_whslr_deliver(const char* item1,const char* item2)
{
	char outmsg[300];
	sprintf(outmsg,"The wholesaler delivers %s and %s\n",item1,item2);
	write(STDOUT_FILENO,outmsg,strlen(outmsg));
}

/**
* Chefs utility functions 
*/
void print_chef_delivered(int chefno)
{
	char outmsg[300];
	sprintf(outmsg,"Chef %d has delivered the %s to the wholesaler\n",chefno,DESERT);
	write(STDOUT_FILENO,outmsg,strlen(outmsg));
}
void prepare_desert(int chefno,int sleeptime)
{
	if(sleeptime <= 0){
		char errmsg[300];
		sprintf(errmsg,"Invalid sleep time given to preapre() method.\n");
		write(STDERR_FILENO,errmsg,strlen(errmsg));
		exit(EXIT_FAILURE);
	}

	char outmsg[300];
	sprintf(outmsg,"Chef %d is preparing the %s\n",chefno,DESERT);
	write(STDOUT_FILENO,outmsg,strlen(outmsg));

	sleep(sleeptime);
}
void post_desert_prepared()
{
	struct sembuf sops[1];

	sops[0].sem_num = 4; // prepared_desert semaphore
	sops[0].sem_op = 1; // wait, subtract 1
	sops[0].sem_flg = 0;

	if(-1 == semop(all_sems,sops,1)){
		char errmsg[300];
		sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
		write(STDERR_FILENO,errmsg,strlen(errmsg));
		exit(EXIT_FAILURE);
	}
}
void print_chef_hastaken(int chefno,const char* item1,const char* item2)
{
	char outmsg[300];
	sprintf(outmsg,"Chef %d has taken %s and %s\n",chefno,item1,item2);
	write(STDOUT_FILENO,outmsg,strlen(outmsg));
}
void print_chef_atwait(int chefno,const char* item1,const char* item2)
{
	char outmsg[300];
	sprintf(outmsg,"Chef %d is waiting for %s and %s\n",chefno,item1,item2);
	write(STDOUT_FILENO,outmsg,strlen(outmsg));
}

void get_item()
{
	strcpy(items_shared,"**"); // means items are popped.
}

void* run_chef(void* arg)
{
	/**
	* Setup cancel states.
	* Firstly, this thread must be CANCELLABLE.
	* This thread will be cancelled by main thread (wholesaler), so must be ASYNCHRONOUS
	*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);

	int chef_no = *((int*)arg);
	struct sembuf sops[2];

	if(chef_no == 1){

		while(TRUE)
		{
			/* wait W and S posted by wholesaler */

			// wait (get) W
			sops[0].sem_num = 2;
			sops[0].sem_op = -1; 
			sops[0].sem_flg = 0;

			// wait (get) S
			sops[1].sem_num = 3;
			sops[1].sem_op = -1; 
			sops[1].sem_flg = 0;

			print_chef_atwait(chef_no,"walnut","sugar");

			if(-1 == semop(all_sems,sops,2)){
				char errmsg[300];
				sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
				write(STDERR_FILENO,errmsg,strlen(errmsg));
				exit(EXIT_FAILURE);
			}

			// pop items from shared array.
			get_item();

			print_chef_hastaken(chef_no,"walnut","sugar");
			prepare_desert(chef_no,rand_select(5));
			print_chef_delivered(chef_no);
			post_desert_prepared();
		}

	}
	else if(chef_no == 2){

		while(TRUE)
		{	
			/* wait F and W posted by wholesaler */
			// wait (get) F
			sops[0].sem_num = 1;
			sops[0].sem_op = -1; 
			sops[0].sem_flg = 0;

			// wait (get) W
			sops[1].sem_num = 2;
			sops[1].sem_op = -1; 
			sops[1].sem_flg = 0;

			print_chef_atwait(chef_no,"flour","walnut");

			if(-1 == semop(all_sems,sops,2)){
				char errmsg[300];
				sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
				write(STDERR_FILENO,errmsg,strlen(errmsg));
				exit(EXIT_FAILURE);
			}

			// pop items from shared array.
			get_item();

			print_chef_hastaken(chef_no,"walnut","sugar");
			prepare_desert(chef_no,rand_select(5));
			print_chef_delivered(chef_no);
			post_desert_prepared();
		}		
	}
	else if(chef_no == 3){
	
		while(TRUE)
		{
			/* wait S and F posted by wholesaler */
			// wait (get) F
			sops[0].sem_num = 1;
			sops[0].sem_op = -1; 
			sops[0].sem_flg = 0;

			// wait (get) S
			sops[1].sem_num = 3;
			sops[1].sem_op = -1; 
			sops[1].sem_flg = 0;

			print_chef_atwait(chef_no,"sugar","flour");

			if(-1 == semop(all_sems,sops,2)){
				char errmsg[300];
				sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
				write(STDERR_FILENO,errmsg,strlen(errmsg));
				exit(EXIT_FAILURE);
			}
			
			// pop items from shared array.
			get_item();

			print_chef_hastaken(chef_no,"walnut","sugar");
			prepare_desert(chef_no,rand_select(5));
			print_chef_delivered(chef_no);
			post_desert_prepared();
		}
	}
	else if(chef_no == 4){
	
		while(TRUE)
		{
			/* wait M and W posted by wholesaler */
			// wait (get) M
			sops[0].sem_num = 0;
			sops[0].sem_op = -1; 
			sops[0].sem_flg = 0;

			// wait (get) W
			sops[1].sem_num = 2;
			sops[1].sem_op = -1; 
			sops[1].sem_flg = 0;

			print_chef_atwait(chef_no,"milk","walnut");

			if(-1 == semop(all_sems,sops,2)){
				char errmsg[300];
				sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
				write(STDERR_FILENO,errmsg,strlen(errmsg));
				exit(EXIT_FAILURE);
			}

			// pop items from shared array.
			get_item();

			print_chef_hastaken(chef_no,"walnut","sugar");
			prepare_desert(chef_no,rand_select(5));
			print_chef_delivered(chef_no);
			post_desert_prepared();
		}
	}
	else if(chef_no == 5){

		while(TRUE)
		{
			/* wait M and S posted by wholesaler */
			// wait (get) M
			sops[0].sem_num = 0;
			sops[0].sem_op = -1; 
			sops[0].sem_flg = 0;

			// wait (get) S
			sops[1].sem_num = 3;
			sops[1].sem_op = -1; 
			sops[1].sem_flg = 0;

			print_chef_atwait(chef_no,"milk","sugar");

			if(-1 == semop(all_sems,sops,2)){
				char errmsg[300];
				sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
				write(STDERR_FILENO,errmsg,strlen(errmsg));
				exit(EXIT_FAILURE);
			}

			// pop items from shared array.
			get_item();

			print_chef_hastaken(chef_no,"walnut","sugar");
			prepare_desert(chef_no,rand_select(5));
			print_chef_delivered(chef_no);
			post_desert_prepared();
		}
	}
	else if(chef_no == 6){

		while(TRUE)
		{
			/* wait M and F posted by wholesaler */
			// wait (get) M
			sops[0].sem_num = 0;
			sops[0].sem_op = -1; 
			sops[0].sem_flg = 0;

			// wait (get) F
			sops[1].sem_num = 1;
			sops[1].sem_op = -1; 
			sops[1].sem_flg = 0;

			print_chef_atwait(chef_no,"milk","flour");

			if(-1 == semop(all_sems,sops,2)){
				char errmsg[300];
				sprintf(errmsg,"Semaphore operation cannot be done (?) : %s\nExit.\n",strerror(errno));
				write(STDERR_FILENO,errmsg,strlen(errmsg));
				exit(EXIT_FAILURE);
			}

			// pop items from shared array.
			get_item();

			print_chef_hastaken(chef_no,"walnut","sugar");
			prepare_desert(chef_no,rand_select(5));
			print_chef_delivered(chef_no);
			post_desert_prepared();
		}
	}

	return NULL;
}

void _init_sigint()
{
	struct sigaction sigint_action;
	memset(&sigint_action,0,sizeof(sigint_action));
	sigint_action.sa_handler = &handler_SIGINT;
	sigaction(SIGINT,&sigint_action,NULL);
}

void _init_semaphores()
{
	char errmsg[300];

	union semun arg;
	
	/** 
	* create 4 semaphores for all type of food and last semaphore 
	* represents the desert posted.
	*/
	all_sems = semget(IPC_PRIVATE,5,0660 | IPC_CREAT);

	if(all_sems == -1){
		sprintf(errmsg,"Semaphore cannot created by semget(). (?) : %s\nExit.\n",strerror(errno));
		write(STDERR_FILENO,errmsg,strlen(errmsg));
		exit(EXIT_FAILURE);
	}

	/* give initial values */
	unsigned short init_sem_vals[5];
	for (int i = 0; i < 5; ++i)
		init_sem_vals[i] = 0;

	arg.array = init_sem_vals;

	if(-1 == semctl(all_sems,0,SETALL,arg)){
		sprintf(errmsg,"Semaphore cannot initialized by semctl(). (?) : %s\nExit.\n",strerror(errno));
		write(STDERR_FILENO,errmsg,strlen(errmsg));
		exit(EXIT_FAILURE);
	}
}
void _clear_semaphores()
{	
	char errmsg[300];

	if(semctl(all_sems,0,IPC_RMID) == -1){
		sprintf(errmsg,"Semaphore cannot cleared by semctl(). (?) : %s\nExit.\n",strerror(errno));
		write(STDERR_FILENO,errmsg,strlen(errmsg));
		exit(EXIT_FAILURE);
	}
}

void _join_chef_threads()
{	
	for (int i = 0; i < NUM_OF_CHEFS; ++i)
		pthread_join(chefs[i],NULL);	
}
void _cancel_chefs()
{
	for (int i = 0; i < NUM_OF_CHEFS; ++i)
		pthread_cancel(chefs[i]);  
}
void _start_chef_threads()
{
	for (int i = 0; i < NUM_OF_CHEFS; ++i)
	{
		chef_nums[i] = i+1;
		pthread_create(&(chefs[i]),NULL,run_chef,&(chef_nums[i]));
	}
}