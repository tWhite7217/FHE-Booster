#include "dag_generator.h"

const int N = 10; // 1000 is the given example on FPBench
// const double t0 = 1.0; // -2 < t0 < 2
// const double w0 = 2.0; // -5 < w0 < 5

int main(int argc, char *argv[])
{
    std::ofstream output_file(argv[1]);

    auto dg = DAGGenerator(output_file);

    dg.set_zero_var();
    dg.set_one_var();
    dg.set_sine9_vars();

    variable t0;
    variable w0;

    dg.print_set(t0);
    dg.print_set(w0);

    variable h;      // 0.01
    variable half_h; // 0.005
    variable m;      // 1.5
    variable gL;     // -9.80665 * 0.5

    dg.print_set(h);
    dg.print_set(half_h);
    dg.print_set(m);
    dg.print_set(gL);

    output_file << "~" << std::endl;

    variable t = t0;
    variable w = w0;

    variable k1w;
    variable k2t;
    variable t_1;
    variable k2w;
    variable w_2;

    variable tmp;

    for (int n = 1; n <= N; n++)
    {
        // double k1w = gL * my_sine(t);
        dg.print_sine9(tmp, t);
        dg.print_mul(k1w, gL, tmp);
        // double k2t = w + ((half_h)*k1w);
        dg.print_mul(tmp, half_h, k1w);
        dg.print_add(k2t, w, tmp);
        // double t_1 = t + (h * k2t);
        dg.print_mul(tmp, h, k2t);
        dg.print_add(t_1, t, tmp);
        // double k2w = gL * my_sine((t + ((half_h)*w)));
        dg.print_mul(tmp, half_h, w);
        dg.print_add(tmp, t, tmp);
        dg.print_sine9(tmp, tmp);
        dg.print_mul(k2w, gL, tmp);
        // double w_2 = w + (h * k2w);
        dg.print_mul(tmp, h, k2w);
        dg.print_add(w_2, w, tmp);
        // t = t_1;
        t = t_1;
        // w = w_2;
        w = w_2;
    }
}