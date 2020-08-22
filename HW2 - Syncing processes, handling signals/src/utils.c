#include "utils.h"


void print_usage()
{
	printf("Usage : ./program [-i] inputPath [-o] outputPath\n");
}

int test_all_args_given(int param_i_test,int param_o_test)
{
	if(!param_i_test){
		printf("[-i] argument not given !\nProgram Terminated\n\n");
		print_usage();
		return 1; 
	}
	if(!param_o_test){
		printf("[-o] argument not given !\nProgram Terminated\n\n");
		print_usage();
		return 1; 
	}
	return 0;
}

int is_digit(char c)
{	
	return !( (int)c < 48 || (int)c > 57); 
}

int parse_args(char * inputPath, char * outputPath,int argc,char *argv[])
{

	int opt;
	int param_i_test=0,param_o_test = 0;
	int file_size;

	while ((opt = getopt(argc, argv,"i:o:")) != -1)
	{	
		switch(opt)
		{	
			case 'i':
				file_size = strlen(optarg);
				if(file_size > MAX_FILE_SIZE){
					printf("Maximum file size is %d, but input file size is %d\nProgram Terminated!\n", MAX_FILE_SIZE,file_size);
					return 1; 
				}
				strcpy(inputPath,optarg);
				param_i_test = 1;
			break;

			case 'o':
				file_size = strlen(optarg);
				if(file_size > MAX_FILE_SIZE){
					printf("Maximum file size is %d, but input file size is %d\nProgram Terminated!\n", MAX_FILE_SIZE,file_size);
					return 1; 
				}
				strcpy(outputPath,optarg);
				param_o_test = 1;	
			break;

			default:
			print_usage();
			printf("Program terminated !\n");
			return 1;
			break;
				
		}
	}	

	return test_all_args_given(param_i_test,param_o_test);
}


void print_point(point_2D p)
{
	char str[POINT_TO_STR_SIZE];
	point_to_string(p,str);
	printf("%s\n", str);
}

char * point_to_string(point_2D p,char * str)
{	
	sprintf(str,"(%d,%d)",p.x,p.y);
	return str;
}


point_2D * line_to_point_arr(unsigned char * line,size_t size)
{
	point_2D * arr = (point_2D*)calloc(BUFF_SIZE / 2,sizeof(point_2D));
	int j = 0;
	for (int i = 0; i < size; i = i+2)
	{	
		/* decode by ASCII now */
		arr[j].x = (int)line[i];
		arr[j].y = (int)line[i+1];
		++j;
	}
	return arr;
}

void point_arr_to_line(point_2D * arr,size_t size,char * res_str)
{
	res_str[0] = '\0';
	for (int i = 0; i < size; ++i)
	{	
		char strtemp[POINT_TO_STR_SIZE];
		point_to_string(arr[i],strtemp);
		strcat(res_str,strtemp);
		if(i != size -1)
			strcat(res_str,",");
	} 
}

point_2D decode_str_to_point(const char * str,int size)
{

	point_2D point;
	
	char x[10];
	char y[10];

	int j = 0;
	int xc = 0;
	for (int i = 1; i < size; ++i)
	{
		if(str[i] == ','){
			while(j<i-1)
			{
				x[j] = str[j+1];
				++j;
			}
			xc = i;
			break;
		}
	}
	xc++;
	x[j] = '\0';
	j = 0;
	for (int i = xc; i < size-1; ++i,++j)
		y[j] = str[i];	

	y[j] = '\0';
	
	point.x = atoi(x);
	point.y = atoi(y);

	return point;
}

/**
* return addres must be deleted.
*/

point_2D * strline_to_point_arr(char * str, size_t size,double * a, double *b)
{

	point_2D * arr = (point_2D*)calloc(BUFF_SIZE / 2, sizeof(point_2D));
	int i = 0;

	char * tokenize;
	char tmp[60];
	int cursor = 0;
	
	char * str_tmp = (char*)calloc(strlen(str)+1,sizeof(char));

	strcpy(str_tmp,str);

	tokenize = strtok(str_tmp,")");

	while(tokenize != NULL)
	{	
		strcpy(tmp,tokenize);
		strcat(tmp,")");
		if(i!=0){
			tmp[0] = '\0';
			strcpy(tmp,&tmp[1]);
		}
		
		arr[i++] = decode_str_to_point(tmp,strlen(tmp));
		cursor += strlen(tmp);
		if(i == size){		
			cursor += size;
			tmp[0] = '\0';
			
			strcpy(tmp,&str_tmp[cursor]);
		
			/* get the line equation*/
			char x[10];
			char y[10];

			int j = 0;
			while(tmp[++j] != 'x');
			int k = 0;
			while(k<j)
			{
				x[k]=tmp[k];
				++k;
			}
			x[k] = '\0';
			
			strcpy(y,&tmp[j+2]);
			

			*a = atof(x);
			*b = atof(y);
			
			break;
		}
		
		tokenize = strtok(NULL,")");

	}
	free(str_tmp);
	return arr;
}

void LSM(point_2D * arr,size_t size,double * x, double * y)
{
	int i;
    int xps[size],yps[size];
    double a,b;
    
    for (i=0;i<size;i++)
        xps[i] = arr[i].x;
   
    for (i=0;i<size;i++)
       yps[i] = arr[i].y;

    double xsum=0,x2sum=0,ysum=0,xysum=0;           
    for (i=0;i<size;i++)
    {	
        xsum=xsum+xps[i];                        
        ysum=ysum+yps[i];                        
        x2sum=x2sum+pow(xps[i],2);                
        xysum=xysum+xps[i]*yps[i];                    
    }
    a=(size*xysum-xsum*ysum)/(size*x2sum-xsum*xsum);           
    b=(x2sum*ysum-xsum*xysum)/(x2sum*size-xsum*xsum);               
    
    *x = a;
    *y = b;
}

double MAE(point_2D * arr, double line_x, double line_y,size_t size)
{
	/* line_x * x + line_y = 0 
	* Ax + b
	*/
	double sum_diff=0;
	for (int i = 0; i < size; ++i)
		sum_diff += abs((line_x*arr[i].x + line_y) - arr[i].y);
	return sum_diff / size;
}

double MSE(point_2D * arr, double line_x, double line_y,size_t size)
{
	/* line_x * x + line_y = 0 
	* Ax + b
	*/
	double sum_diff=0;
	double square;
	for (int i = 0; i < size; ++i){
		square = (line_x*arr[i].x + line_y) - arr[i].y;
		sum_diff += square * square;
	}
	return sum_diff / size;
}

double RMSE(point_2D * arr, double line_x, double line_y,size_t size)
{
	return sqrt(MSE(arr,line_x,line_y,size));
}

