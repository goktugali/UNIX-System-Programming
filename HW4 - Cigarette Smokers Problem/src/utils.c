#include "utils.h"

int rand_select(int bound)
{
    return 1 + (rand() % bound);
}

void print_usage()
{
	printf("Usage : ../main -i filepath\n");
}

int test_all_args_given(int param_f_test)
{
    if(!param_f_test){
        printf("[-i] argument not given !\nProgram Terminated\n\n");
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
char * read_file(const char* filepath)
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

	int line_size = char_count(readed_file,'\n',size);

	char * parsed_file = (char*)calloc(sizeof(char),(line_size*2) + 1);
	char * token = strtok(readed_file,"\n");
	while(token != NULL)
	{	
		if(!is_valid_pair(token) || strlen(token) != 2){
			free(parsed_file);
			free(readed_file);
			_exit_invalid_file(filepath);
		}
		strcat(parsed_file,token);
		token = strtok(NULL,"\n");
	}
	free(readed_file);
	return parsed_file;
}

int char_count(const char* string, char c,size_t size)
{	
	int ctr = 0;
	for (int i = 0; i < size; ++i)
		if(string[i] == c)
			++ctr;
	return ctr + 1;
}

int is_valid_pair(char pair[2]){

	return pair[0] != pair[1] && 
	((pair[0] == 'M' || pair[0] == 'F' || pair[0] == 'W' || pair[0] == 'S') && 
		(pair[1] == 'M' || pair[1] == 'F' || pair[1] == 'W' || pair[1] == 'S')); 
}
void _exit_invalid_file(const char* filepath)
{
	char errmsg[300];
	sprintf(errmsg,"File '%s' is not a valid file !\n",filepath);
	write(STDERR_FILENO,errmsg,strlen(errmsg));
	exit(EXIT_FAILURE);
}