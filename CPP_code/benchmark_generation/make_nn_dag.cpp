#include "dag_generator.h"

// const int in_size = 28 * 28;
// const int l1out_neurons = 100;
// const int l2out_neurons = 10;
const int in_size = 28 * 28;
const int l1out_neurons = 20;
const int l2out_neurons = 10;
// const int in_size = 4 * 4;
// const int l1out_neurons = 6;
// const int l2out_neurons = 5;

int main(int argc, char *argv[])
{
    std::ofstream output_file(argv[1]);

    auto dg = DAGGenerator(output_file);

    std::array<variable, l1out_neurons> b1;
    std::array<variable, l2out_neurons> b2;

    std::array<variable, in_size> in;
    std::array<variable, in_size> w;

    dg.print_set_array(b1);
    dg.print_set_array(b2);
    dg.print_set_array(in);
    dg.print_set_array(w);

    output_file << "~" << std::endl;

    variable l1out[l1out_neurons];
    variable res[l2out_neurons];

    variable tmp;

    for (auto j = 0; j < l1out_neurons; j++)
    {
        l1out[j] = b1[j];
        for (auto i = 0; i < in_size; i++)
        {
            dg.print_mul(tmp, in[i], w[i]);
            dg.print_add(l1out[j], l1out[j], tmp);
        }
    }

    for (auto i = 0; i < l1out_neurons; i++)
    {
        dg.print_mul(l1out[i], l1out[i]);
    }

    for (auto j = 0; j < l2out_neurons; j++)
    {
        res[j] = b2[j];
        for (auto i = 0; i < l1out_neurons; i++)
        {
            dg.print_mul(tmp, l1out[i], w[i]);
            dg.print_add(res[j], res[j], tmp);
        }
    }
}