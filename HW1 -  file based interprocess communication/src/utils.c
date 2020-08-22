#include "utils.h"


void internal_file_test()
{

	int ipc_fd = open(IPC_FILE,O_RDONLY);
	int temp_fd = open(TEMP_FILE,O_RDONLY);

	if(ipc_fd < 0){
		fprintf(stderr, "%s file nout found!\nCheck the internal program txt files..!\n", IPC_FILE);
		exit(EXIT_FAILURE);
	}

	if(temp_fd < 0){
		fprintf(stderr, "%s file nout found!\nCheck the internal program txt files..!\n", TEMP_FILE);
		exit(EXIT_FAILURE);
	}
}

void print_usage()
{
	printf("Usage : ./program [-i] inputPath [-o] outputPath [-t] time\n");
}

void complex_to_string(char *str,complex_num num)
{	
	
	sprintf(str,"%.0f + i%.0f",num.real,num.img);
	
}

void complex_to_string_double(char *str,complex_num num)
{

	sprintf(str,"%.3f + i%.3f",num.real,num.img);
}

complex_num decode_ASCII(char str[2])
{
	complex_num num;
	num.img = (int)str[1];
	num.real = (int)str[0];

	return num;
}

/**
* return adress must be deleted !
*/
char * complex_arr_to_string(const complex_num* numbers,int size,char type)
{

	char * res_str = (char*)calloc(MAX_COMPLEX_STR_SIZE*size,sizeof(char));
	char tmp_str[MAX_COMPLEX_STR_SIZE];

	for (int i = 0; i < size; ++i)
	{	
		if(type == 'i') // print integer
			complex_to_string(tmp_str,numbers[i]);
		else if(type == 'd')
			complex_to_string_double(tmp_str,numbers[i]);
		else{
			fprintf(stderr, "Invalid type given as argmument 3 in function : complex_arr_to_string(numbers,%d,%c)\n", size,type);
			exit(EXIT_FAILURE);
		}
		if(i!=0)
			strcat(res_str,",");
		strcat(res_str,tmp_str);
	}

	return res_str;
}



/**
* return addres must be deleted !
*/

complex_num* str_to_complex_arr(const char * str, int buff_size)
{	
	int j = 0;
	char tmp[2];
	int arr_ctr = 0;
	complex_num * numbers = (complex_num*)calloc(buff_size / 2,sizeof(complex_num));

	for (int i = 0; i < buff_size; ++i)
	{
		tmp[j++] = str[i];
		if(j==2){
			numbers[arr_ctr++] = decode_ASCII(tmp);
			j = 0;
		}
	}

	return numbers;
}

complex_num decode_str_to_complex(const char * str,int size)
{

	complex_num num;
	
	char real[10];
	char img[10];

	int j = 0;
	int x = 0;
	for (int i = 0; i < size; ++i)
	{
		if(str[i] == '+'){
			while(j<i-1)
			{
				real[j] = str[j];
				++j;
			}
			x = i;
			break;
		}
	}
	x = x+3;
	real[j] = '\0';
	j = 0;
	for (int i = x; i < size; ++i,++j)
		img[j] = str[i];	

	img[j] = '\0';
	
	num.real = atoi(real);
	num.img = atoi(img);

	return num;
}

/**
* return addres must be deleted.
*/

complex_num * strline_to_complex_arr(char * str)
{

	complex_num * arr = (complex_num*)calloc(BUFF_SIZE / 2, sizeof(complex_num));
	int i = 0;

	char * tokenize;
	tokenize = strtok(str,",");
	while(tokenize != NULL)
	{
		arr[i++] = decode_str_to_complex(tokenize,strlen(tokenize));
		tokenize = strtok(NULL,",");	
	}
	
	return arr;
}



int test_all_args_given(int param_i_test,int param_o_test,int param_t_test)
{
	if(!param_i_test){
		printf("[-i] argument not given !\nProgram Terminated\n\n");
		print_usage();
		return 1; // error
	}
	if(!param_o_test){
		printf("[-o] argument not given !\nProgram Terminated\n\n");
		print_usage();
		return 1; // error
	}
	if(!param_t_test){
		printf("[-t] argument not given !\nProgram Terminated\n\n");
		print_usage();
		return 1; // error
	}
	return 0;
}

int is_digit(char c)
{
	return !( (int)c < 48 || (int)c > 57); 
}

file_stat file_status()
{

	int fd = open(IPC_FILE,O_RDWR);
	file_stat status;
	if(fd <0 ){
		fprintf(stderr, "%s cannot opened for file comm.!\nProgram terminated\n", IPC_FILE);
		exit(EXIT_FAILURE);
	}

	struct flock lock;
	memset(&lock,0,sizeof(lock));

	lock.l_type = F_RDLCK;
	fcntl(fd,F_SETLKW,&lock); /* lock the ipc file */

	char read_str[6]= "------";
	read(fd,read_str,6);
	read_str[5] = '\0';

	if(strcmp(read_str,"-----") == 0){
		status.A_CNT = -1;
		status.B_CNT = -1;
			
		lock.l_type = F_UNLCK;
		fcntl(fd,F_SETLKW,&lock); /* unlock the ipc file */
		return status;
	}

	int A_CNT = -1;
	int B_CNT = -1;
	
	if(read_str[0] == 'A' || read_str[0] == 'a')
		if(is_digit(read_str[1]))
			A_CNT = (int)read_str[1] - '0';
	
	if(read_str[3] == 'B' || read_str[3] == 'b')
		if(is_digit(read_str[4]))
			B_CNT = (int)read_str[4] - '0';


	status.A_CNT = A_CNT;
	status.B_CNT = B_CNT;

	lock.l_type = F_UNLCK;
	fcntl(fd,F_SETLKW,&lock); /* lock the ipc file */
	close(fd);


	return status;
}

int parse_args(char * inputPath, char * outputPath, int * time,int argc,char *argv[])
{

	int opt;
	int param_i_test,param_o_test,param_t_test = 0;
	int file_size;


	while ((opt = getopt(argc, argv,"i:o:t:")) != -1)
	{	
		switch(opt)
		{	
			case 'i':
				file_size = strlen(optarg);
				if(file_size > MAX_FILE_SIZE){
					printf("Maximum file size is %d, but input file size is %d\nProgram Terminated!\n", MAX_FILE_SIZE,file_size);
					return 1; // error
				}
				strcpy(inputPath,optarg);
				param_i_test = 1;
			break;

			case 'o':
				file_size = strlen(optarg);
				if(file_size > MAX_FILE_SIZE){
					printf("Maximum file size is %d, but input file size is %d\nProgram Terminated!\n", MAX_FILE_SIZE,file_size);
					return 1; // error
				}
				strcpy(outputPath,optarg);
				param_o_test = 1;	
			break;

			case 't':
				*time = atoi(optarg);
				if(*time < 1 || *time > 50){
					printf("Invalid [-t] time input : %d\nTime [-t] must be in [1,50]\n", *time);
					return 1; // error
				}
				param_t_test = 1;
			break;

			default:
			print_usage();
			printf("Program terminated !\n");
			return 1; // error
			break;
				
		}
	}	

	return test_all_args_given(param_i_test,param_o_test,param_t_test);
}
void update_file_status(file_stat status)
{

	int fd = open(IPC_FILE,O_RDWR);
	if(fd <0 ){
		fprintf(stderr, "%s cannot opened (IPC).!\nProgram terminated", IPC_FILE);
		exit(1);
	}

	struct flock lock;
	memset(&lock,0,sizeof(lock));

	lock.l_type = F_WRLCK;
	fcntl(fd,F_SETLKW,&lock); /* lock the ipc file */

	fd = open(IPC_FILE,O_RDWR | O_TRUNC);

	char str[5];

	str[0] = 'A';
	str[1] = status.A_CNT + '0';
	str[2] = '\n';
	str[3] = 'B';
	str[4] = status.B_CNT + '0';
	write(fd,str,5);

	lock.l_type = F_WRLCK;
	fcntl(fd,F_SETLKW,&lock); /* lock the ipc file */

	close(fd);

}