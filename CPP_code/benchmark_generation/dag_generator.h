#include <fstream>
#include <string>
#include <array>

using variable = std::string;
using bit = std::string;

class DAGGenerator
{
public:
    DAGGenerator(std::ofstream &);

    void print_set(variable &);
    void print_add(variable &, variable &, variable &);
    void print_sub(variable &, variable &, variable &);
    void print_mul(variable &, variable &);
    void print_mul(variable &, variable &, variable &);

    template <size_t N>
    void print_set_array(std::array<variable, N> &);

    void set_zero();

    template <size_t N>
    void set(std::array<bit, N> &, std::array<bit, N> &);
    template <size_t N>
    void print_rotate_left(std::array<bit, N> &, std::array<bit, N> &, int);
    template <size_t N>
    void print_rotate_right(std::array<bit, N> &, std::array<bit, N> &, int);
    template <size_t N>
    void shift_left(std::array<bit, N> &, std::array<bit, N> &, int);
    template <size_t N>
    void shift_right(std::array<bit, N> &, std::array<bit, N> &, int);
    template <size_t N>
    void print_set(std::array<bit, N> &);
    template <size_t N>
    void print_xor(std::array<bit, N> &, std::array<bit, N> &, std::array<bit, N> &);
    template <size_t N>
    void print_or(std::array<bit, N> &, std::array<bit, N> &, std::array<bit, N> &);
    template <size_t N>
    void print_and(std::array<bit, N> &, std::array<bit, N> &, std::array<bit, N> &);

private:
    size_t constant_count = 1;
    size_t instruction_count = 1;
    // size_t num_bits;

    bit zero;

    std::ofstream &output_file;
};

template <size_t N>
void DAGGenerator::print_set_array(std::array<variable, N> &output_arr)
{
    for (auto &var : output_arr)
    {
        print_set(var);
    }
}

template <size_t N>
void DAGGenerator::set(std::array<bit, N> &output_arr, std::array<bit, N> &input_arr)
{
    output_arr = input_arr;
}

template <size_t N>
void DAGGenerator::print_rotate_left(std::array<bit, N> &output_arr, std::array<bit, N> &input_arr, int num)
{
    std::array<bit, N> t1mp;
    shift_left(t1mp, input_arr, num);

    std::array<bit, N> t2mp;
    shift_right(t2mp, input_arr, N - num);

    std::array<bit, N> t3mp;
    print_or(output_arr, t1mp, t2mp);
}

template <size_t N>
void DAGGenerator::print_rotate_right(std::array<bit, N> &output_arr, std::array<bit, N> &input_arr, int num)
{
    std::array<bit, N> t1mp;
    shift_right(t1mp, input_arr, num);

    std::array<bit, N> t2mp;
    shift_left(t2mp, input_arr, N - num);

    std::array<bit, N> t3mp;
    print_or(output_arr, t1mp, t2mp);
}

template <size_t N>
void DAGGenerator::shift_left(std::array<bit, N> &output_arr, std::array<bit, N> &input_arr, int num)
{
    for (int i = 0; i < N; i++)
    {
        if (i < num)
        {
            output_arr[i] = zero;
        }
        else
        {
            output_arr[i] = input_arr[i - num];
        }
    }
}

template <size_t N>
void DAGGenerator::shift_right(std::array<bit, N> &output_arr, std::array<bit, N> &input_arr, int num)
{
    for (int i = 0; i < N; i++)
    {
        if (i + num < N)
        {
            output_arr[i] = input_arr[i + num];
        }
        else
        {
            output_arr[i] = zero;
        }
    }
}

template <size_t N>
void DAGGenerator::print_set(std::array<bit, N> &output_arr)
{
    for (int i = 0; i < N; i++)
    {
        output_file << constant_count++ << ",SET" << std::endl;
        output_arr[i] = "k" + std::to_string(constant_count - 1);
    }
}

template <size_t N>
void DAGGenerator::print_or(std::array<bit, N> &output_arr, std::array<bit, N> &input_arr1, std::array<bit, N> &input_arr2)
{
    for (int i = 0; i < N; i++)
    {
        bit c1 = input_arr1[i];
        bit c2 = input_arr2[i];
        output_file << instruction_count++ << ",MUL," << c1 << "," << c2 << std::endl;
        output_file << instruction_count++ << ",ADD," << c1 << "," << c2 << std::endl;
        output_file << instruction_count++ << ",SUB,c" << instruction_count - 3 << ",c" << instruction_count - 2 << std::endl;
        output_arr[i] = "c" + std::to_string(instruction_count - 1);
    }
}

template <size_t N>
void DAGGenerator::print_and(std::array<bit, N> &output_arr, std::array<bit, N> &input_arr1, std::array<bit, N> &input_arr2)
{
    for (int i = 0; i < N; i++)
    {
        bit c1 = input_arr1[i];
        bit c2 = input_arr2[i];
        output_file << instruction_count++ << ",MUL," << c1 << "," << c2 << std::endl;
        output_arr[i] = "c" + std::to_string(instruction_count - 1);
    }
}

template <size_t N>
void DAGGenerator::print_xor(std::array<bit, N> &output_arr, std::array<bit, N> &input_arr1, std::array<bit, N> &input_arr2)
{
    for (int i = 0; i < N; i++)
    {
        bit c1 = input_arr1[i];
        bit c2 = input_arr2[i];
        output_file << instruction_count++ << ",SUB," << c1 << "," << c2 << std::endl;
        output_file << instruction_count++ << ",MUL,c" << instruction_count - 2 << std::endl;
        output_arr[i] = "c" + std::to_string(instruction_count - 1);
    }
}