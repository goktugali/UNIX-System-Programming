/**
* CSE344 SYSTEMS PROGRAMMING
* HOMEWORK 1
* GOKTUG ALI AKIN | 161044018
*
* THIS FILE CONTAINS UTILITY FUNCTIONS TO
* CALCULATING THE DISCRETE FOURIER TRANSFORM
*/

#ifndef FFT_H
#define FFT_H

#include <math.h>
#include <stdlib.h>
#include "fft.h"
#include "complex.h"

#define MAX 200


int l_log2(int N);
int check(int n);
int reverse(int N, int n);
void ordina(complex_num* f1, int N);

void transform(complex_num* f, int N);
void FFT(complex_num* f, int N, double d);


#endif