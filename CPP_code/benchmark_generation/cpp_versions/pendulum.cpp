#include <fenv.h>
#include <math.h>
#include <stdint.h>
#include <iostream>

double my_sine(double x)
{
	return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0);
	// return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0) + (pow(x, 9) / 362880.0);
	// return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0) + (pow(x, 9) / 362880.0) - (pow(x, 11) / 39916800.0);
	// return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0) + (pow(x, 9) / 362880.0) - (pow(x, 11) / 39916800.0) + (pow(x, 13) / 6227020800.0);
	// return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0) + (pow(x, 9) / 362880.0) - (pow(x, 11) / 39916800.0) + (pow(x, 13) / 6227020800.0) - (pow(x, 15) / 1307674368000.0);
	// return sin(x);
}

double ex0(double t0, double w0, double N)
{
	const double h = 0.01;
	const double half_h = 0.005;
	// const double L = 0.5;
	double m = 1.5;
	// const double g = -9.80665;
	const double gL = -9.80665 * 0.5;
	double t = t0;
	double w = w0;
	for (int n = 1; n <= N; n++)
	{
		double k1w = gL * my_sine(t);
		double k2t = w + ((half_h)*k1w);
		double t_1 = t + (h * k2t);
		double k2w = gL * my_sine((t + ((half_h)*w)));
		double w_2 = w + (h * k2w);
		t = t_1;
		w = w_2;
		// std::cout << n << std::endl;
	}
	std::cout << t << std::endl;
	return t;
}

int main()
{
	ex0(1.0, -2.0, 1000);
}