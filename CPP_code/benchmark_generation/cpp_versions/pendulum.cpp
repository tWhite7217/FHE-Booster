#include <fenv.h>
#include <math.h>
#include <stdint.h>
#include <iostream>

double my_sine(double x)
{
	// return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0);
	// return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0) + (pow(x, 9) / 362880.0);
	// return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0) + (pow(x, 9) / 362880.0) - (pow(x, 11) / 39916800.0);
	// return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0) + (pow(x, 9) / 362880.0) - (pow(x, 11) / 39916800.0) + (pow(x, 13) / 6227020800.0);
	// return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0) + (pow(x, 9) / 362880.0) - (pow(x, 11) / 39916800.0) + (pow(x, 13) / 6227020800.0) - (pow(x, 15) / 1307674368000.0);

	// double x2 = x * x;
	// double x3 = x2 * x;
	// double x5 = x3 * x2;
	// double x6 = x3 * x3;
	// double x7 = x5 * x2;
	// double x9 = x6 * x3;
	// double x11 = x6 * x5;
	// return ((x - (x3 / 6.0)) + (x5 / 120.0)) - (x7 / 5040.0) + (x9 / 362880.0) - (x11 / 39916800.0);
	// return ((x - (pow(x, 3) * 1.666666666666666667e-1)) + (pow(x, 5) * 8.333333333333333333e-3)) - (pow(x, 7) * 1.984126984126984127e-4) + (pow(x, 9) * 2.755731922398589065e-6) - (pow(x, 11) * 2.505210838544171878e-8);
	// return ((x - (pow(x, 3) * 0.166667)) + (pow(x, 5) * 0.00833333)) - (pow(x, 7) * 0.000198413) + (pow(x, 9) * 2.75573e-06) - (pow(x, 11) * 2.50521e-08);

	return sin(x);
}

double ex0(double t0, double w0, double N)
{
	const double h = 0.01;
	const double half_h = 0.005;
	const double gL = -4.903325;
	double t = t0;
	double w = w0;
	for (int n = 1; n <= N; n++)
	{
		double k1w = gL * my_sine(t);
		double k2t = w + (half_h * k1w);
		double k2w = gL * my_sine(t + (half_h * w));
		t = t + (h * k2t);
		w = w + (h * k2w);
	}
	std::cout << "w: " << w << std::endl;
	std::cout << "t: " << t << std::endl;
	return t;
}

int main()
{
	ex0(1.0, 2.0, 12);
}