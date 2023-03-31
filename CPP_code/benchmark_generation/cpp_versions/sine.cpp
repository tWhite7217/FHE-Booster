#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x)
{
	// return ((x - (((x * x) * x) / 6.0)) + (((((x * x) * x) * x) * x) / 120.0)) - (((((((x * x) * x) * x) * x) * x) * x) / 5040.0);
	return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0);
	// return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0) + (pow(x, 9) / 362880.0);
	// return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0) + (pow(x, 9) / 362880.0) - (pow(x, 11) / 39916800.0);
}