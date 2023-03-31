#include "dag_generator.h"

DAGGenerator::DAGGenerator(std::ofstream &output_file) : output_file{output_file}
{
    // output_file << "ADD,1" << std::endl;
    // output_file << "SUB,1" << std::endl;
    // output_file << "MUL,5" << std::endl;
    // output_file << "~" << std::endl;
}

void DAGGenerator::print_set(variable &output)
{
    output_file << constant_count++ << ",SET" << std::endl;
    output = "k" + std::to_string(constant_count - 1);
}

void DAGGenerator::print_add(variable &output, const variable &input1, const variable &input2)
{
    output_file << instruction_count++ << ",ADD," << input1 << "," << input2 << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}

void DAGGenerator::print_sub(variable &output, const variable &input1, const variable &input2)
{
    output_file << instruction_count++ << ",SUB," << input1 << "," << input2 << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}

void DAGGenerator::print_mul(variable &output, const variable &input)
{
    output_file << instruction_count++ << ",MUL," << input << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}

void DAGGenerator::print_mul(variable &output, const variable &input1, const variable &input2)
{
    output_file << instruction_count++ << ",MUL," << input1 << "," << input2 << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}

void DAGGenerator::print_pow(variable &output, const variable &input, int exp)
{
    switch (exp)
    {
    case 5:
        return print_pow5(output, input);
    case 7:
        return print_pow7(output, input);
    case 9:
        return print_pow9(output, input);
    case 11:
        return print_pow11(output, input);
    case 13:
        return print_pow11(output, input);
    case 15:
        return print_pow11(output, input);
    }

    variable result = one_var;
    variable base = input;
    for (;;)
    {
        if (exp & 1)
        {
            print_mul(result, result, base);
        }
        exp >>= 1;
        if (!exp)
            break;
        print_mul(base, base, base);
    }

    output = result;
}

void DAGGenerator::print_set_vector(std::vector<variable> &output_vec)
{
    for (auto &var : output_vec)
    {
        print_set(var);
    }
}

void DAGGenerator::set_zero_bit()
{
    if (zero_bit.empty())
    {
        zero_bit = "k" + std::to_string(constant_count);
        output_file << constant_count++ << ",SET" << std::endl;
    }
}

void DAGGenerator::set_zero_var()
{
    if (zero_var.empty())
    {
        zero_var = "k" + std::to_string(constant_count);
        output_file << constant_count++ << ",SET" << std::endl;
    }
}

void DAGGenerator::set_one_var()
{
    if (one_var.empty())
    {
        one_var = "k" + std::to_string(constant_count);
        output_file << constant_count++ << ",SET" << std::endl;
    }
}

void DAGGenerator::print_pow5(variable &output, const variable &a)
{
    variable b;
    variable tmp;

    // (a × a → b) × b × a
    print_mul(b, a, a);
    print_mul(tmp, b, b);
    print_mul(output, tmp, a);
}

void DAGGenerator::print_pow7(variable &output, const variable &a)
{
    variable b;
    variable tmp1;
    variable tmp2;

    // (a × a → b) × b × b × a
    print_mul(b, a, a);
    print_mul(tmp1, b, b);
    print_mul(tmp2, tmp1, b);
    print_mul(output, tmp2, a);
}

void DAGGenerator::print_pow9(variable &output, const variable &a)
{
    variable tmp1;
    variable c;
    variable tmp2;

    // (a × a × a → c) × c × c
    print_mul(tmp1, a, a);
    print_mul(c, tmp1, a);
    print_mul(tmp2, c, c);
    print_mul(output, tmp2, c);
}

void DAGGenerator::print_pow11(variable &output, const variable &a)
{
    variable b;
    variable d;
    variable tmp1;
    variable tmp2;

    // ((a × a → b) × b → d) × d × b × a
    print_mul(b, a, a);
    print_mul(d, b, b);
    print_mul(tmp1, d, d);
    print_mul(tmp2, tmp1, b);
    print_mul(output, tmp2, a);
}

void DAGGenerator::print_pow13(variable &output, const variable &a)
{
    variable b;
    variable d;
    variable tmp1;
    variable tmp2;

    // ((a × a → b) × b → d) × d × d × a
    print_mul(b, a, a);
    print_mul(d, b, b);
    print_mul(tmp1, d, d);
    print_mul(tmp2, tmp1, d);
    print_mul(output, tmp2, a);
}

void DAGGenerator::print_pow15(variable &output, const variable &a)
{
    variable b;
    variable e;
    variable tmp1;
    variable tmp2;

    // ((a × a → b) × b × a → e) × e × e
    print_mul(b, a, a);
    print_mul(tmp1, b, b);
    print_mul(e, tmp1, a);
    print_mul(tmp2, e, e);
    print_mul(output, tmp2, e);
}

void DAGGenerator::print_sine7(variable &output, const variable &x)
{
    variable tmp1;
    variable tmp2;
    variable tmp3;

    // return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0);
    print_pow(tmp1, x, 3);
    print_mul(tmp1, tmp1, inv6);
    print_sub(tmp1, x, tmp1);

    print_pow(tmp2, x, 5);
    print_mul(tmp2, tmp2, inv120);

    print_pow(tmp3, x, 7);
    print_mul(tmp3, tmp3, inv5040);

    print_add(tmp1, tmp1, tmp2);
    print_sub(output, tmp1, tmp3);
}

void DAGGenerator::print_sine9(variable &output, const variable &x)
{
    variable tmp1;
    variable tmp2;

    print_sine7(tmp1, x);

    print_pow(tmp2, x, 9);
    print_mul(tmp2, tmp2, inv362880);
    print_add(output, tmp1, tmp2);
}

void DAGGenerator::print_sine11(variable &output, const variable &x)
{
    variable tmp1;
    variable tmp2;

    print_sine9(tmp1, x);

    print_pow(tmp2, x, 11);
    print_mul(tmp2, tmp2, inv39916800);
    print_sub(output, tmp1, tmp2);
}

// void DAGGenerator::print_sine13(variable &output, const variable &x)
// {
//     variable tmp1;
//     variable tmp2;

//     print_sine11(tmp1, x);

//     print_pow(tmp2, x, 13);
//     print_mul(tmp2, tmp2, 1 / 6227020800);
//     print_add(output, tmp1, tmp2);
// }

// void DAGGenerator::print_sine15(variable &output, const variable &x)
// {
//     variable tmp1;
//     variable tmp2;

//     print_sine13(tmp1, x);

//     print_pow(tmp2, x, 15);
//     print_mul(tmp2, tmp2, 1 / 1307674368000);
//     print_sub(output, tmp1, tmp2);
// }

void DAGGenerator::set_sine7_vars()
{
    if (inv6.empty())
    {
        inv6 = "k" + std::to_string(constant_count);
        output_file << constant_count++ << ",SET" << std::endl;

        inv120 = "k" + std::to_string(constant_count);
        output_file << constant_count++ << ",SET" << std::endl;

        inv5040 = "k" + std::to_string(constant_count);
        output_file << constant_count++ << ",SET" << std::endl;
    }
}

void DAGGenerator::set_sine9_vars()
{
    set_sine7_vars();

    if (inv362880.empty())
    {
        inv362880 = "k" + std::to_string(constant_count);
        output_file << constant_count++ << ",SET" << std::endl;
    }
}

void DAGGenerator::set_sine11_vars()
{
    set_sine9_vars();

    if (inv39916800.empty())
    {
        inv39916800 = "k" + std::to_string(constant_count);
        output_file << constant_count++ << ",SET" << std::endl;
    }
}

// void DAGGenerator::set_sine13_vars();

// void DAGGenerator::set_sine15_vars();