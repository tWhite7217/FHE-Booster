#include <fenv.h>
#include <math.h>
#include <stdint.h>
#include <iostream>
#define TRUE 1
#define FALSE 0

double ex0(double m, double kp, double ki, double kd, double c)
{
	const int num_iterations = 20;

	double dt = 0.2;
	double invdt = 1.0 / dt;
	double e = 0.0;
	double p = 0.0;
	double i = 0.0;
	double d = 0.0;
	double r = 0.0;
	double eold = 0.0;

	for (int iter = 0; iter < num_iterations; iter++)
	{
		e = c - m;
		p = kp * e;
		i = i + ((ki * dt) * e);
		d = (kd * invdt) * (e - eold);
		r = (p + i) + d;
		m = m + (0.01 * r);
		eold = e;

		std::cout << "kp: " << kp << std::endl;
		std::cout << "ki: " << ki << std::endl;
		std::cout << "kd: " << kd << std::endl;
		std::cout << "c: " << c << std::endl;
		std::cout << "dt: " << dt << std::endl;
		std::cout << "invdt: " << invdt << std::endl;
		std::cout << "e: " << e << std::endl;
		std::cout << "p: " << p << std::endl;
		std::cout << "i: " << i << std::endl;
		std::cout << "d: " << d << std::endl;
		std::cout << "r: " << r << std::endl;
		std::cout << "eold: " << eold << std::endl;
		std::cout << std::endl;
		std::cout << "m: " << m << std::endl;
		std::cout << std::endl;
	}

	return m;
}

int main()
{
	const double m = -5.0;
	const double kp = 9.4514;
	const double ki = 0.69006;
	const double kd = 2.8454;
	const double c = 1.0;
	ex0(m, kp, ki, kd, c);
}