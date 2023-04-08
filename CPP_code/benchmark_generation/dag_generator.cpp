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
    std::string inputs = get_inputs_string(input1, input2);
    output_file << instruction_count++ << ",ADD," << inputs << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}

std::string DAGGenerator::get_inputs_string(const variable &input1, const variable &input2)
{
    if (input1 == input2)
    {
        return input1;
    }
    return input1 + "," + input2;
}

void DAGGenerator::print_sub(variable &output, const variable &input1, const variable &input2)
{
    output_file << instruction_count++ << ",SUB," << input1 << "," << input2 << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}

void DAGGenerator::print_mul(variable &output, const variable &input1, const variable &input2)
{
    std::string inputs = get_inputs_string(input1, input2);
    output_file << instruction_count++ << ",MUL," << inputs << std::endl;
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
    variable x2;
    variable x3;
    variable x5;
    variable x7;

    // return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0);
    print_mul(x2, x, x);
    print_mul(x3, x2, x);
    print_mul(x5, x3, x2);
    print_mul(x7, x5, x2);

    print_mul(tmp1, x3, inv6);
    print_sub(tmp1, x, tmp1);

    print_mul(tmp2, x5, inv120);

    print_mul(tmp3, x7, inv5040);

    print_add(tmp1, tmp1, tmp2);
    print_sub(output, tmp1, tmp3);
}

void DAGGenerator::print_sine9(variable &output, const variable &x)
{
    variable tmp1;
    variable tmp2;
    variable tmp3;
    variable tmp4;
    variable x2;
    variable x3;
    variable x5;
    variable x6;
    variable x7;
    variable x9;

    // return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0) + (pow(x, 9) / 362880.0);
    print_mul(x2, x, x);
    print_mul(x3, x2, x);
    print_mul(x5, x3, x2);
    print_mul(x6, x3, x3);
    print_mul(x7, x5, x2);
    // One extra multiplication by calculating x9 this way, but more parallelism and
    // all these multiplcations use 4 levels, whereas doing x7 * x2 would add a
    // level, for a total of 5
    print_mul(x9, x6, x3);

    print_mul(tmp1, x3, inv6);
    print_sub(tmp1, x, tmp1);

    print_mul(tmp2, x5, inv120);

    print_mul(tmp3, x7, inv5040);

    print_mul(tmp4, x9, inv362880);

    print_add(tmp1, tmp1, tmp2);
    print_sub(tmp1, tmp1, tmp3);
    print_add(output, tmp1, tmp4);
}

void DAGGenerator::print_sine11(variable &output, const variable &x)
{
    variable tmp1;
    variable tmp2;
    variable tmp3;
    variable tmp4;
    variable tmp5;
    variable x2;
    variable x3;
    variable x5;
    variable x6;
    variable x7;
    variable x9;
    variable x11;

    // return ((x - (pow(x, 3) / 6.0)) + (pow(x, 5) / 120.0)) - (pow(x, 7) / 5040.0) + (pow(x, 9) / 362880.0) - (pow(x, 11) / 39916800.0);
    print_mul(x2, x, x);
    print_mul(x3, x2, x);
    print_mul(x5, x3, x2);
    print_mul(x6, x3, x3);
    print_mul(x7, x5, x2);
    print_mul(x9, x6, x3);
    print_mul(x11, x6, x5);
    // Again, all these multiplications only use 4 levels

    print_mul(tmp1, x3, inv6);
    print_sub(tmp1, x, tmp1);

    print_mul(tmp2, x5, inv120);

    print_mul(tmp3, x7, inv5040);

    print_mul(tmp4, x9, inv362880);

    print_mul(tmp5, x11, inv39916800);

    print_add(tmp1, tmp1, tmp2);
    print_sub(tmp1, tmp1, tmp3);
    print_add(tmp1, tmp1, tmp4);
    print_sub(output, tmp1, tmp5);
}

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