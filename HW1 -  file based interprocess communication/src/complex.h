/**
* CSE344 SYSTEMS PROGRAMMING
* HOMEWORK 1
* GOKTUG ALI AKIN | 161044018
*
* COMPLEX STRUCT FOR REPRESENTING THE COMPLEX NUMBER
* THIS FILE CONTAINS UTILITY FUNCTIONS THE MANIPULATE THE COMPLEX NUMBER
*/

#ifndef COMPLEX_H
#define COMPLEX_H

#include <math.h>
#include <stdio.h>

typedef struct complexnumber
{
	double real;
	double img;

}complex_num;


/**
* toString printer for complex number
*/
void print_complex(complex_num cn1);

/**
* return complex number with given maginutde and phase angle
*/
complex_num polar(double magnitude, double theta);

/**
* add two complex number
* operator+ for complex
*/
complex_num complex_add(complex_num side1,complex_num side2);

/**
* sub two complex number
* operator- for complex
*/
complex_num complex_sub(complex_num side1,complex_num side2);

/**
* product two complex number
* operator* for complex
*/
complex_num complex_product(complex_num side1,complex_num side2);

/**
* returns the power of complex number.
*/
complex_num pow_complex(complex_num num, int power);

#endif