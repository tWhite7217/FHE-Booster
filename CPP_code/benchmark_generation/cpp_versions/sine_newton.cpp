#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x0)
{
	double x = x0;
	double i = 0.0;
	int tmp = i < 10.0;
	while (tmp)
	{
		double x_1 = x - ((((x - (pow(x, 3.0) / 6.0)) + (pow(x, 5.0) / 120.0)) + (pow(x, 7.0) / 5040.0)) / (((1.0 - ((x * x) / 2.0)) + (pow(x, 4.0) / 24.0)) + (pow(x, 6.0) / 720.0)));
		double i_2 = i + 1.0;
		x = x_1;
		i = i_2;
		tmp = i < 10.0;
		std::cout << i << std::endl;
	}
	std::cout << x << std::endl;
	return x;
}

int main()
{
	ex0(3.14);
}