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

void DAGGenerator::print_add(variable &output, variable &input1, variable &input2)
{
    output_file << instruction_count++ << ",ADD," << input1 << "," << input2 << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}

void DAGGenerator::print_sub(variable &output, variable &input1, variable &input2)
{
    output_file << instruction_count++ << ",SUB," << input1 << "," << input2 << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}

void DAGGenerator::print_mul(variable &output, variable &input)
{
    output_file << instruction_count++ << ",MUL," << input << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}

void DAGGenerator::print_mul(variable &output, variable &input1, variable &input2)
{
    output_file << instruction_count++ << ",MUL," << input1 << "," << input2 << std::endl;
    output = "c" + std::to_string(instruction_count - 1);
}

void DAGGenerator::set_zero()
{
    zero = "k" + std::to_string(constant_count);
    output_file << constant_count++ << ",SET" << std::endl;
}