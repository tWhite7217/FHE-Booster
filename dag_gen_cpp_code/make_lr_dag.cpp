#include <iostream>
#include <fstream>
#include <string>
#include <array>

// const int num_inferences = 10;
const int num_attributes = 16;

int constant_count = 1;
int instruction_count = 1;

std::ofstream output_file;

using variable = std::string;

template <size_t N>
void print_set_array(std::array<variable, N> &);
void print_set(variable &);
void print_add(variable &, variable &, variable &);
void print_sub(variable &, variable &, variable &);
void print_mul(variable &, variable &);
void print_mul(variable &, variable &, variable &);

int main(int argc, char *argv[])
{
    output_file.open(argv[1]);

    output_file << "ADD,1" << std::endl;
    output_file << "SUB,1" << std::endl;
    output_file << "MUL,5" << std::endl;
    output_file << "~" << std::endl;

    std::array<variable, num_attributes> weights;
    // std::array<variable, num_inferences> accum;
    variable accum;
    variable out;

    std::array<variable, num_attributes> inputs;

    print_set_array(weights);
    print_set(accum);
    print_set_array(inputs);
    print_set(out);
    // print_set_array(accum);
    // for (auto arr : inputs)
    // {
    //     print_set_array(arr);
    // }

    output_file << "~" << std::endl;

    // Multiply all attributes by the pre-trained weights and accumulate
    for (auto i = 0; i < num_attributes; i++)
    {
        print_mul(inputs[i], inputs[i], weights[i]);
        print_add(accum, accum, inputs[i]);
    }

    variable temp;
    variable tmp;

    // Sigmoid (high accuracy taylor-series approximation)
    print_mul(tmp, accum);
    print_add(out, out, tmp);
    // Calculate x^3 term.
    print_mul(temp, accum);
    print_mul(temp, temp, accum);
    print_mul(tmp, temp);
    print_sub(out, out, tmp);
    // Calculate x^5 term.
    print_mul(temp, temp, accum);
    print_mul(temp, temp, accum);
    print_mul(tmp, temp);
    print_sub(out, out, tmp);
    // Calculate x^9 term.
    print_mul(temp, temp, accum);
    print_mul(temp, temp, accum);
    print_mul(tmp, temp);
    print_sub(out, out, tmp);
    // Calculate x^11 term.
    print_mul(temp, temp, accum);
    print_mul(temp, temp, accum);
    print_mul(tmp, temp);
    print_sub(out, out, tmp);
}

template <size_t N>
void print_set_array(std::array<variable, N> &output_arr)
{
    for (auto &var : output_arr)
    {
        print_set(var);
    }
}

void print_set(variable &output)
{
    output_file << constant_count++ << ",SET" << std::endl;
    output = "k" + std::to_string(constant_count - 1);
}

void print_add(variable &output, variable &input1, variable &input2)
{
    output_file << instruction_count++ << ",ADD," << input1 << "," << input2 << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}

void print_sub(variable &output, variable &input1, variable &input2)
{
    output_file << instruction_count++ << ",SUB," << input1 << "," << input2 << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}

void print_mul(variable &output, variable &input)
{
    output_file << instruction_count++ << ",MUL," << input << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}

void print_mul(variable &output, variable &input1, variable &input2)
{
    output_file << instruction_count++ << ",MUL," << input1 << "," << input2 << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}