#include <fenv.h>
#include <math.h>
#include <stdint.h>
#include <iostream>
#define TRUE 1
#define FALSE 0

double ex0(double m, double kp, double ki, double kd, double c)
{
	double dt = 0.5;
	double invdt = 1.0 / dt;
	double e = 0.0;
	double p = 0.0;
	double i = 0.0;
	double d = 0.0;
	double r = 0.0;
	double m_1 = m;
	double eold = 0.0;
	double t = 0.0;
	int tmp = t < 100.0;

	while (tmp)
	{
		e = c - m_1;
		p = kp * e;
		i = i + ((ki * dt) * e);
		d = (kd * invdt) * (e - eold);
		r = (p + i) + d;
		m_1 = m_1 + (0.01 * r);
		eold = e;
		t = t + dt;
		tmp = t < 100.0;
	}

	std::cout << "m: " << m << std::endl;
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
	std::cout << "m_1: " << m_1 << std::endl;
	std::cout << "eold: " << eold << std::endl;
	std::cout << "t: " << t << std::endl;

	return m_1;
}

int main()
{
	ex0(-5.0, 9.4514, 2.8454, 0.69006, 7.0);
}