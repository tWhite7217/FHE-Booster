#include <iostream>
#include <fstream>
#include <string>
#include <array>

const int in_size = 28 * 28;
const int l1out_neurons = 100;
const int l2out_neurons = 10;
// const int in_size = 28 * 28;
// const int l1out_neurons = 20;
// const int l2out_neurons = 10;
// const int in_size = 4 * 4;
// const int l1out_neurons = 6;
// const int l2out_neurons = 5;

int constant_count = 1;
int instruction_count = 1;

std::ofstream output_file;

using variable = std::string;

template <size_t N>
void print_set_array(std::array<variable, N> &);
void print_set(variable &);
void print_add(variable &, variable &, variable &);
void print_mul(variable &, variable &);
void print_mul(variable &, variable &, variable &);

int main(int argc, char *argv[])
{
    output_file.open(argv[1]);

    output_file << "ADD,1" << std::endl;
    output_file << "SUB,1" << std::endl;
    output_file << "MUL,5" << std::endl;
    output_file << "~" << std::endl;

    std::array<variable, l1out_neurons> b1;
    std::array<variable, l2out_neurons> b2;

    std::array<variable, in_size> in;
    std::array<variable, in_size> w;

    print_set_array(b1);
    print_set_array(b2);
    print_set_array(in);
    print_set_array(w);

    output_file << "~" << std::endl;

    variable l1out[l1out_neurons];
    variable res[l2out_neurons];

    variable tmp;

    for (auto j = 0; j < l1out_neurons; j++)
    {
        l1out[j] = b1[j];
        for (auto i = 0; i < in_size; i++)
        {
            print_mul(tmp, in[i], w[i]);
            print_add(l1out[j], l1out[j], tmp);
        }
    }

    for (auto i = 0; i < l1out_neurons; i++)
    {
        print_mul(l1out[i], l1out[i]);
    }

    for (auto j = 0; j < l2out_neurons; j++)
    {
        res[j] = b2[j];
        for (auto i = 0; i < l1out_neurons; i++)
        {
            print_mul(tmp, l1out[i], w[i]);
            print_add(res[j], res[j], tmp);
        }
    }
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