#include "fft.h"

int l_log2(int N)    /*function to calculate the l_log2(.) of int numbers*/
{
  	int k = N, i = 0;
  	while(k) {
	    k >>= 1;
	    i++;
  	}
  	return i - 1;
}

int check(int n)    //checking if the number of element is a power of 2
{
  return n > 0 && (n & (n - 1)) == 0;
}

int reverse(int N, int n)    //calculating revers number
{
	int j, p = 0;
	for(j = 1; j <= l_log2(N); j++)
		if(n & (1 << (l_log2(N) - j)))
	    	p |= 1 << (j - 1);	
	return p;
}

void ordina(complex_num* f1, int N) //using the reverse order in the array
{
	complex_num f2[MAX];
	for(int i = 0; i < N; i++)
		f2[i] = f1[reverse(N, i)];
	for(int j = 0; j < N; j++)
		f1[j] = f2[j];
}

void transform(complex_num* f, int N) //
{
	ordina(f, N);    //first: reverse order
	complex_num *W;
	W = (complex_num *)malloc(N / 2 * sizeof(complex_num));
	W[1] = polar(1., -2. * M_PI / N);
	complex_num num;
	num.real = 1.0;
	num.img = 0;
	W[0] = num;
	for(int i = 2; i < N / 2; i++)
		W[i] = pow_complex(W[1], i);

	int n = 1;
	int a = N / 2;
	for(int j = 0; j < log2(N); j++) 
	{
		for(int i = 0; i < N; i++) 
		{
			if(!(i & n)){
				complex_num temp = f[i];
				complex_num Temp = complex_product(W[(i * a) % (n * a)],f[i + n]);
				f[i] = complex_add(temp,Temp);
				f[i + n] = complex_sub(temp,Temp);
			}
		}
	n *= 2;
	a = a / 2;
	}
	free(W);
}


void FFT(complex_num* f, int N, double d)
{
	transform(f, N);
	for(int i = 0; i < N; i++)
	{	
		complex_num num;
		num.real = d;
		num.img = 0;
		f[i] = complex_product(f[i],num);
	}
}