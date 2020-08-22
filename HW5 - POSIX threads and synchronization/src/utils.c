#include "utils.h"

int rand_select(int bound)
{
    return 1 + (rand() % bound);
}

int sleep_ms(long msec)
{
	struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

void print_usage()
{
	char info_msg[50];
    sprintf(info_msg,"Usage : ../floristApp -i filepath\n");
    write(STDERR_FILENO,info_msg,strlen(info_msg));
}

int test_all_args_given(int param_f_test)
{
    if(!param_f_test){
    	char info_msg[50];
    	sprintf(info_msg,"[-i] argument not given !\nProgram Terminated\n\n");
    	write(STDERR_FILENO,info_msg,strlen(info_msg));
        print_usage();
        return 1; // error
    }
	return 0;
}
int parse_args(char* filepath,int argc,char *argv[])
{

	int opt;
	int param_f_test=0;
   
	while ((opt = getopt(argc, argv,"i:")) != -1)
	{	
		switch(opt)
		{	
            case 'i':
            sprintf(filepath,"%s",optarg);
            param_f_test = 1;
            break;

			default:
			print_usage();
			printf("Program terminated !\n");
			return 1; // error
			break;
				
		}
	}	
	return test_all_args_given(param_f_test);
}
char ** read_file(const char* filepath,int* lines,int* client_start)
{	
	char errmsg[300];
	int fd = open(filepath,O_RDONLY);
	if(fd == -1){
		sprintf(errmsg,"File '%s' cannot opened ! (?) : %s \n",filepath,strerror(errno));
		write(STDERR_FILENO,errmsg,strlen(errmsg));
		exit(EXIT_FAILURE);
	}

	size_t size = lseek(fd,0,SEEK_END);
	if(size <= 0){
		close(fd);
		_exit_invalid_file(filepath);
	}

	lseek(fd,0,SEEK_SET);

	char * readed_file = (char*)calloc(sizeof(char),(size+1));
	read(fd,readed_file,size);
	readed_file[size] = '\0';
	close(fd);

	int line_size = char_count(readed_file,'\n',size) + 1;

	char** parsed_file = (char**)calloc(sizeof(char*),line_size);
	
	int index = 0;
	int client_found = 0;
	char * token = strtok(readed_file,"\n");
	while(token != NULL)
	{	
		parsed_file[index] = (char*)calloc(sizeof(char),strlen(token)+1);
		strcpy(parsed_file[index],token);
		
		if(!client_found && char_count(parsed_file[index],';',strlen(parsed_file[index])) == 0){
			*client_start = index;
			client_found = 1;
		}

		++index;
		token = strtok(NULL,"\n");
	}
	free(readed_file);
	*lines = index;
	return parsed_file;	
}

int char_count(const char* string, char c,size_t size)
{	
	int ctr = 0;
	for (int i = 0; i < size; ++i)
		if(string[i] == c)
			++ctr;
	return ctr;
}

char* remove_whitespace(const char* token)
{
	char* token_cpy = (char*)calloc(sizeof(char),strlen(token) + 1);
	
	int index = 0;
	for (int i = 0; i < strlen(token); ++i)
	{
		if(token[i] != ' '){
			token_cpy[index] = token[i];
			++index;
		}
	}

	return token_cpy;
}

void _exit_invalid_file(const char* filepath)
{
	char errmsg[300];
	sprintf(errmsg,"File '%s' is not a valid file !\n",filepath);
	write(STDERR_FILENO,errmsg,strlen(errmsg));
	exit(EXIT_FAILURE);
}

point_t str_to_point(const char* token)
{

	char* token_cpy = (char*)calloc(sizeof(char),strlen(token) + 1);
	char* bckp_addr = token_cpy;
	strcpy(token_cpy,token);

	token_cpy = &token_cpy[1];
	token_cpy[strlen(token_cpy)-1] = '\0';

	point_t res_pt;

	char* token_pt = strtok_r(token_cpy,",",&token_cpy);

	res_pt.x = atof(token_pt);
	token_pt = strtok_r(token_cpy,",",&token_cpy);
	res_pt.y = atof(token_pt);

	free(bckp_addr);
	return  res_pt;
}

florist_t str_to_florist(const char* token)
{	

	florist_t res_fl;

	char* token_cpy = (char*)calloc(sizeof(char),strlen(token) + 1);
	char* bckp_addr = token_cpy;
	strcpy(token_cpy,token);

	char* token_name = strtok_r(token_cpy," ",&token_cpy);
	res_fl.name = (char*)calloc(sizeof(char),strlen(token_name) + 1);
	strcpy(res_fl.name,token_name);

	token_cpy = remove_whitespace(token_cpy);
	free(bckp_addr);
	bckp_addr = token_cpy;

	char* token_vals = strtok_r(token_cpy,":",&token_cpy);
	char* token_pt = strtok_r(token_vals,";",&token_vals);
	char* tmp_token = (char*)calloc(sizeof(char),strlen(token_pt) + 2);
	strcpy(tmp_token,token_pt);
	strcat(tmp_token,")");
	
	point_t pt = str_to_point(tmp_token);
	res_fl.pt = pt;
	free(tmp_token);

	token_vals[strlen(token_vals) - 1] = '\0';

	res_fl.speed = atof(token_vals);

	int num_items = char_count(token_cpy,',',strlen(token_cpy)) + 1;

	res_fl.items = (char**)calloc(sizeof(char**),num_items);
	res_fl.num_items = num_items;

	char* token_item = strtok_r(token_cpy,",",&token_cpy);
	int index = 0;
	while(token_item != NULL)
	{	
		res_fl.items[index] = (char*)calloc(sizeof(char),strlen(token_item) + 1);
		strcpy(res_fl.items[index],token_item);
		++index;
		token_item = strtok_r(token_cpy,",",&token_cpy);
	}

	res_fl.total_time = 0;

	free(bckp_addr);
	return res_fl;
}

client_t str_to_client(const char* token)
{
	client_t res_client;

	char* token_cpy = (char*)calloc(sizeof(char),strlen(token) + 1);
	char* bckp_addr = token_cpy;
	strcpy(token_cpy,token);

	char* token_name = strtok_r(token_cpy," ",&token_cpy);
	res_client.name = (char*)calloc(sizeof(char),strlen(token_name) + 1);
	strcpy(res_client.name,token_name);

	token_cpy = remove_whitespace(token_cpy);
	free(bckp_addr);
	bckp_addr = token_cpy;

	char* token_rest = strtok_r(token_cpy,":",&token_cpy);

	res_client.pt = str_to_point(token_rest);

	res_client.requested_item = (char*)calloc(sizeof(char),strlen(token_cpy) + 1);
	strcpy(res_client.requested_item,token_cpy);

	free(bckp_addr);

	return res_client;
}


request_queue* create_queue(unsigned int size)
{
	request_queue* res_q = (request_queue*)calloc(sizeof(request_queue),size);
	res_q->capacity = size;
	res_q->items = (client_t*)calloc(sizeof(client_t),size);
	res_q->front = 0;
	res_q->size = 0;
	res_q->rear = res_q->capacity - 1;
	return res_q;
}

void queue_push(request_queue* src_queue, client_t item)
{
	src_queue->rear = (src_queue->rear + 1) % src_queue -> capacity;
	src_queue->items[src_queue->rear] = item;
	src_queue->size = 1 + src_queue->size; 
}

client_t queue_pop(request_queue* src_queue)
{	
	client_t item = src_queue->items[src_queue->front];
    src_queue->front = (src_queue->front + 1) % src_queue->capacity;
    src_queue->size = src_queue->size - 1; 
    return item;
}