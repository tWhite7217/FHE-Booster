#include "dag_generator.h"

// const int num_inferences = 10;
const int num_attributes = 16;

int main(int argc, char *argv[])
{
    std::ofstream output_file(argv[1]);

    auto dg = DAGGenerator(output_file);

    std::array<variable, num_attributes> weights;
    variable accum;
    variable out;

    std::array<variable, num_attributes> inputs;

    dg.print_set_array(weights);
    dg.print_set(accum);
    dg.print_set_array(inputs);
    dg.print_set(out);

    output_file << "~" << std::endl;

    // Multiply all attributes by the pre-trained weights and accumulate
    for (auto i = 0; i < num_attributes; i++)
    {
        dg.print_mul(inputs[i], inputs[i], weights[i]);
        dg.print_add(accum, accum, inputs[i]);
    }

    variable temp;
    variable tmp;

    // Sigmoid (high accuracy taylor-series approximation)
    dg.print_mul(tmp, accum);
    dg.print_add(out, out, tmp);
    // Calculate x^3 term.
    dg.print_mul(temp, accum);
    dg.print_mul(temp, temp, accum);
    dg.print_mul(tmp, temp);
    dg.print_sub(out, out, tmp);
    // Calculate x^5 term.
    dg.print_mul(temp, temp, accum);
    dg.print_mul(temp, temp, accum);
    dg.print_mul(tmp, temp);
    dg.print_sub(out, out, tmp);
    // Calculate x^9 term.
    dg.print_mul(temp, temp, accum);
    dg.print_mul(temp, temp, accum);
    dg.print_mul(tmp, temp);
    dg.print_sub(out, out, tmp);
    // Calculate x^11 term.
    dg.print_mul(temp, temp, accum);
    dg.print_mul(temp, temp, accum);
    dg.print_mul(tmp, temp);
    dg.print_sub(out, out, tmp);
}