#include "complex.h"


void print_complex(complex_num cn1)
{
	printf("%.3f + %.3fi\n", cn1.real,cn1.img);
}

complex_num polar(double magnitude, double theta)
{
	complex_num num;
	num.real = magnitude*cos(theta);
	num.img = magnitude*sin(theta);
	return num;
}

complex_num complex_add(complex_num side1,complex_num side2)
{
	
	complex_num result;
	result.real = side1.real + side2.real;
	result.img = side1.img + side2.img;
	return result;

	return result;
}
complex_num complex_sub(complex_num side1,complex_num side2)
{
	complex_num result;
	result.real = side1.real - side2.real;
	result.img = side1.img - side2.img;
	return result;
}
complex_num complex_product(complex_num side1,complex_num side2)
{

	complex_num result;
	result.real = (side1.real*side2.real) - (side1.img*side2.img);
	result.img = (side1.real*side2.img) + (side1.img*side2.real);
	return result;

}

complex_num pow_complex(complex_num num, int power)
{

	complex_num result = num;
	for (int i = 0; i < power-1; ++i)
		result = complex_product(result,result);
	return result;
}
