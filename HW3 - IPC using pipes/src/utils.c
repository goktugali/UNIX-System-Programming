#include "utils.h"


void print_usage()
{
	printf("Usage : ./program [-i] inputPathA [-j] inputPathB [-n] size\n");
}

int test_all_args_given(int param_i_test,int param_o_test,int param_t_test)
{
	if(!param_i_test){
		printf("[-i] argument not given !\nProgram Terminated\n\n");
		print_usage();
		return 1; // error
	}
	if(!param_o_test){
		printf("[-j] argument not given !\nProgram Terminated\n\n");
		print_usage();
		return 1; // error
	}
	if(!param_t_test){
		printf("[-n] argument not given !\nProgram Terminated\n\n");
		print_usage();
		return 1; // error
	}
	return 0;
}
int parse_args(char * inputPathA, char * inputPathB, int * n,int argc,char *argv[])
{

	int opt;
	int param_i_test,param_o_test,param_t_test = 0;
	int file_size;

	while ((opt = getopt(argc, argv,"i:j:n:")) != -1)
	{	
		switch(opt)
		{	
			case 'i':
				file_size = strlen(optarg);
				if(file_size > MAX_FILE_SIZE){
					printf("Maximum file size is %d, but input file size is %d\nProgram Terminated!\n", MAX_FILE_SIZE,file_size);
					return 1; // error
				}
				strcpy(inputPathA,optarg);
				param_i_test = 1;
			break;

			case 'j':
				file_size = strlen(optarg);
				if(file_size > MAX_FILE_SIZE){
					printf("Maximum file size is %d, but input file size is %d\nProgram Terminated!\n", MAX_FILE_SIZE,file_size);
					return 1; // error
				}
				strcpy(inputPathB,optarg);
				param_o_test = 1;	
			break;

			case 'n':
				*n = atoi(optarg);
				if(*n < 1){
					printf("Invalid [-n] input : %d\nn [-n] must be in [1, .. \n", *n);
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

int * string_to_matrice(unsigned char * source_str,int size)
{

	int * resmat = (int*)calloc(size,sizeof(int));
	for (int i = 0; i < size; ++i)
		resmat[i] = (int)source_str[i];
	return resmat;
}

void print_matrice(int * matrice, int row, int column)
{
	for (int i = 0; i < column; ++i)
	{
		for (int j = 0; j < row; ++j){
			char str[10];
			sprintf(str,"%-3d ",matrice[i*row + j]);
			NO_EINTR(write(1,str,strlen(str)));
		}
		NO_EINTR(write(1,"\n",1));
	}
}

void get_child1_inputs(const int * mat_a , const int * mat_b, int * res_input1, int * res_input2, int quarter_size)
{
	
	int x = 0;
	for (int i = 0; i < quarter_size / 2; ++i)
		for (int j = 0; j < quarter_size; ++j)
			res_input1[x++] = mat_a[j + i*quarter_size];	
	x = 0;
	for (int i = 0; i < quarter_size; ++i)
		for (int j = 0; j < quarter_size / 2; ++j)
			res_input2[x++] = mat_b[j + i*quarter_size];

}
void get_child2_inputs(const int * mat_a , const int * mat_b, int * res_input1, int * res_input2, int quarter_size)
{
	int x = 0;
	for (int i = 0; i < quarter_size / 2; ++i)
		for (int j = 0; j < quarter_size; ++j)
			res_input1[x++] = mat_a[j + i*quarter_size];

	x = 0;	
	for (int i = 0; i < quarter_size; ++i)
		for (int j = quarter_size / 2 ; j < quarter_size; ++j)
			res_input2[x++] = mat_b[j + i*quarter_size];
}
void get_child3_inputs(const int * mat_a , const int * mat_b, int * res_input1, int * res_input2, int quarter_size)
{
	int x = 0;
	for (int i = quarter_size/2; i < quarter_size; ++i)
		for (int j = 0 ; j < quarter_size; ++j)
			res_input1[x++] = mat_a[j + i*quarter_size];
		
	
	x = 0;
	for (int i = 0; i < quarter_size; ++i)
		for (int j = 0; j < quarter_size / 2; ++j)
			res_input2[x++] = mat_b[j + i*quarter_size];
}
void get_child4_inputs(const int * mat_a , const int * mat_b, int * res_input1, int * res_input2, int quarter_size)
{
	int x = 0;
	for (int i = quarter_size/2; i < quarter_size; ++i)
		for (int j = 0 ; j < quarter_size; ++j)
			res_input1[x++] = mat_a[j + i*quarter_size];
	x = 0;
	for (int i = 0; i < quarter_size; ++i)
		for (int j = quarter_size / 2 ; j < quarter_size; ++j)
			res_input2[x++] = mat_b[j + i*quarter_size];
}


void convert_1D_to_2D(const int * source_arr, float ** res_arr,int row, int column)
{
	for (int i = 0; i < column; ++i)
		for (int j = 0; j < row; ++j)
			res_arr[i][j] = source_arr[i + j*column];
}


/** SVD CALCULATORS **/

static double PYTHAG(double a, double b)
{
    double at = fabs(a), bt = fabs(b), ct, result;

    if (at > bt)       { ct = bt / at; result = at * sqrt(1.0 + ct * ct); }
    else if (bt > 0.0) { ct = at / bt; result = bt * sqrt(1.0 + ct * ct); }
    else result = 0.0;
    return(result);
}

int dsvd(float **a, int m, int n, float *w, float **v)
{
    int flag, i, its, j, jj, k, l, nm;
    double c, f, h, s, x, y, z;
    double anorm = 0.0, g = 0.0, scale = 0.0;
    double *rv1;
  
    if (m < n) 
    {
        fprintf(stderr, "#rows must be > #cols \n");
        return(0);
    }
  
    rv1 = (double *)malloc((unsigned int) n*sizeof(double));

/* Householder reduction to bidiagonal form */
    for (i = 0; i < n; i++) 
    {
        /* left-hand reduction */
        l = i + 1;
        rv1[i] = scale * g;
        g = s = scale = 0.0;

        if (i < m) 
        {
            for (k = i; k < m; k++) 
                scale += fabs((double)a[k][i]);
            if (scale) 
            {
                for (k = i; k < m; k++) 
                {	 
                    a[k][i] = (float)((double)a[k][i]/scale);
                    s += ((double)a[k][i] * (double)a[k][i]);
                }
                f = (double)a[i][i];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                a[i][i] = (float)(f - g);
                if (i != n - 1) 
                {
                    for (j = l; j < n; j++) 
                    {
                        for (s = 0.0, k = i; k < m; k++) 
                            s += ((double)a[k][i] * (double)a[k][j]);
                        f = s / h;
                        for (k = i; k < m; k++) 
                            a[k][j] += (float)(f * (double)a[k][i]);
                    }
                }
                for (k = i; k < m; k++) 
                    a[k][i] = (float)((double)a[k][i]*scale);
            }
        }
        w[i] = (float)(scale * g);
    
        /* right-hand reduction */
        g = s = scale = 0.0;
        if (i < m && i != n - 1) 
        {
            for (k = l; k < n; k++) 
                scale += fabs((double)a[i][k]);
            if (scale) 
            {
                for (k = l; k < n; k++) 
                {
                    a[i][k] = (float)((double)a[i][k]/scale);
                    s += ((double)a[i][k] * (double)a[i][k]);
                }
                f = (double)a[i][l];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                a[i][l] = (float)(f - g);
                for (k = l; k < n; k++) 
                    rv1[k] = (double)a[i][k] / h;
                if (i != m - 1) 
                {
                    for (j = l; j < m; j++) 
                    {
                        for (s = 0.0, k = l; k < n; k++) 
                            s += ((double)a[j][k] * (double)a[i][k]);
                        for (k = l; k < n; k++) 
                            a[j][k] += (float)(s * rv1[k]);
                    }
                }
                for (k = l; k < n; k++) 
                    a[i][k] = (float)((double)a[i][k]*scale);
            }
        }
        anorm = MAX(anorm, (fabs((double)w[i]) + fabs(rv1[i])));
    }
  
    /* accumulate the right-hand transformation */
    for (i = n - 1; i >= 0; i--) 
    {
        if (i < n - 1) 
        {
            if (g) 
            {
                for (j = l; j < n; j++)
                    v[j][i] = (float)(((double)a[i][j] / (double)a[i][l]) / g);
                    /* double division to avoid underflow */
                for (j = l; j < n; j++) 
                {
                    for (s = 0.0, k = l; k < n; k++) 
                        s += ((double)a[i][k] * (double)v[k][j]);
                    for (k = l; k < n; k++) 
                        v[k][j] += (float)(s * (double)v[k][i]);
                }
            }
            for (j = l; j < n; j++) 
                v[i][j] = v[j][i] = 0.0;
        }
        v[i][i] = 1.0;
        g = rv1[i];
        l = i;
    }
  
    /* accumulate the left-hand transformation */
    for (i = n - 1; i >= 0; i--) 
    {
        l = i + 1;
        g = (double)w[i];
        if (i < n - 1) 
            for (j = l; j < n; j++) 
                a[i][j] = 0.0;
        if (g) 
        {
            g = 1.0 / g;
            if (i != n - 1) 
            {
                for (j = l; j < n; j++) 
                {
                    for (s = 0.0, k = l; k < m; k++) 
                        s += ((double)a[k][i] * (double)a[k][j]);
                    f = (s / (double)a[i][i]) * g;
                    for (k = i; k < m; k++) 
                        a[k][j] += (float)(f * (double)a[k][i]);
                }
            }
            for (j = i; j < m; j++) 
                a[j][i] = (float)((double)a[j][i]*g);
        }
        else 
        {
            for (j = i; j < m; j++) 
                a[j][i] = 0.0;
        }
        ++a[i][i];
    }

    /* diagonalize the bidiagonal form */
    for (k = n - 1; k >= 0; k--) 
    {                             /* loop over singular values */
        for (its = 0; its < 30; its++) 
        {                         /* loop over allowed iterations */
            flag = 1;
            for (l = k; l >= 0; l--) 
            {                     /* test for splitting */
                nm = l - 1;
                if (fabs(rv1[l]) + anorm == anorm) 
                {
                    flag = 0;
                    break;
                }
                if (fabs((double)w[nm]) + anorm == anorm) 
                    break;
            }
            if (flag) 
            {
                c = 0.0;
                s = 1.0;
                for (i = l; i <= k; i++) 
                {
                    f = s * rv1[i];
                    if (fabs(f) + anorm != anorm) 
                    {
                        g = (double)w[i];
                        h = PYTHAG(f, g);
                        w[i] = (float)h; 
                        h = 1.0 / h;
                        c = g * h;
                        s = (- f * h);
                        for (j = 0; j < m; j++) 
                        {
                            y = (double)a[j][nm];
                            z = (double)a[j][i];
                            a[j][nm] = (float)(y * c + z * s);
                            a[j][i] = (float)(z * c - y * s);
                        }

                    }
                }
            }
            z = (double)w[k];
            if (l == k) 
            {                  /* convergence */
                if (z < 0.0) 
                {              /* make singular value nonnegative */
                    w[k] = (float)(-z);
                    for (j = 0; j < n; j++) 
                        v[j][k] = (-v[j][k]);
                }
                break;
            }
            if (its >= 30) {
                free((void*) rv1);
                fprintf(stderr, "No convergence after 30,000! iterations \n");
                return(0);
            }
    
            /* shift from bottom 2 x 2 minor */
            x = (double)w[l];
            nm = k - 1;
            y = (double)w[nm];
            g = rv1[nm];
            h = rv1[k];
            f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
            g = PYTHAG(f, 1.0);
            f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
          
            /* next QR transformation */
            c = s = 1.0;
            for (j = l; j <= nm; j++) 
            {
                i = j + 1;
                g = rv1[i];
                y = (double)w[i];
                h = s * g;
                g = c * g;
                z = PYTHAG(f, h);
                rv1[j] = z;
                c = f / z;
                s = h / z;
                f = x * c + g * s;
                g = g * c - x * s;
                h = y * s;
                y = y * c;
                for (jj = 0; jj < n; jj++) 
                {
                    x = (double)v[jj][j];
                    z = (double)v[jj][i];
                    v[jj][j] = (float)(x * c + z * s);
                    v[jj][i] = (float)(z * c - x * s);
                }
                z = PYTHAG(f, h);
                w[j] = (float)z;
                if (z) 
                {
                    z = 1.0 / z;
                    c = f * z;
                    s = h * z;
                }
                f = (c * g) + (s * y);
                x = (c * y) - (s * g);
                for (jj = 0; jj < m; jj++) 
                {
                    y = (double)a[jj][j];
                    z = (double)a[jj][i];
                    a[jj][j] = (float)(y * c + z * s);
                    a[jj][i] = (float)(z * c - y * s);

                }
            }
            rv1[l] = 0.0;
            rv1[k] = f;
            w[k] = (float)x;
        }
    }
    free((void*) rv1);

    return(1);
}