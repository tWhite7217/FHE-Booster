#include "dag_generator.h"

const int num_iterations = 20; // should be 200 to match the benchmark

int main(int argc, char *argv[])
{
    std::ofstream output_file(argv[1]);

    auto dg = DAGGenerator(output_file);

    dg.set_zero_var();

    variable m;  // Ex. -5.0          -10 < m < 10
    variable kp; // Ex. 9.4514
    variable ki; // Ex. 0.69006
    variable kd; // Ex. 2.8454
    variable c;  // No example given  -10 < c < 10

    dg.print_set(m);
    dg.print_set(kp);
    dg.print_set(ki);
    dg.print_set(kd);
    dg.print_set(c);

    variable dt;    // 0.2 matches the original papers, 0.5 matches FPBench
    variable invdt; // 1 / dt
    variable dot01; // 0.01

    dg.print_set(dt);
    dg.print_set(invdt);
    dg.print_set(dot01);

    output_file << "~" << std::endl;

    variable e = dg.zero_var;
    variable p = dg.zero_var;
    variable i = dg.zero_var;
    variable d = dg.zero_var;
    variable r = dg.zero_var;
    variable eold = dg.zero_var;
    variable t = dg.zero_var;

    variable tmp1;
    variable tmp2;

    for (int iter = 0; iter < num_iterations; iter++)
    {
        // e = c - m;
        dg.print_sub(e, c, m);
        // p = kp * e;
        dg.print_mul(p, kp, e);
        // i = i + ((ki * dt) * e);
        dg.print_mul(tmp1, ki, dt);
        dg.print_mul(tmp2, tmp1, e);
        dg.print_add(i, i, tmp2);
        // d = (kd * invdt) * (e - eold);
        dg.print_mul(tmp1, kd, invdt);
        dg.print_sub(tmp2, e, eold);
        dg.print_mul(d, tmp1, tmp2);
        // r = (p + i) + d;
        dg.print_add(tmp1, p, i);
        dg.print_add(r, tmp1, d);
        // m = m + (0.01 * r);
        dg.print_mul(tmp1, dot01, r);
        dg.print_add(m, m, tmp1);
        // eold = e;
        eold = e;
    }
}