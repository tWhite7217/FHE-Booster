#include <iostream>

double my_sine(const double x)
{
	const double r1 = 1.666666666666666667e-1;
	const double r2 = 8.333333333333333333e-3;
	const double r3 = 1.984126984126984127e-4;
	const double r4 = 2.755731922398589065e-6;
	const double r5 = 2.505210838544171878e-8;

	const double x2 = x * x;
	const double x3 = x2 * x;
	const double x5 = x3 * x2;
	const double x6 = x3 * x3;
	const double x7 = x5 * x2;
	const double x9 = x6 * x3;
	const double x11 = x6 * x5;

	return x - (x3 * r1) + (x5 * r2) - (x7 * r3) + (x9 * r4) - (x11 * r5);
}

double ex0(const double t0, const double w0, const double N)
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