#include <iostream>
#include <fstream>

// const int in_size = 28 * 28;
// const int l1out_neurons = 20;
// const int l2out_neurons = 10;
const int in_size = 4 * 4;
const int l1out_neurons = 6;
const int l2out_neurons = 5;

int main()
{
    std::ofstream output_file;
    output_file.open("test.txt");

    output_file << "ADD,1" << std::endl;
    output_file << "MUL,5" << std::endl;
    output_file << "~" << std::endl;

    int instruction_count = 1;

    for (int j = 0; j < l1out_neurons; j++)
    {
        output_file << instruction_count++ << ",MUL" << std::endl;
        output_file << instruction_count++ << ",ADD," << instruction_count - 2 << std::endl;

        for (int i = 1; i < in_size; i++)
        {
            output_file << instruction_count++ << ",MUL" << std::endl;
            output_file << instruction_count++ << ",ADD," << instruction_count - 3 << "," << instruction_count - 2 << std::endl;
        }
    }

    for (int i = 0; i < l1out_neurons; i++)
    {
        output_file << instruction_count++ << ",MUL," << (in_size * (i + 1) * 2) << std::endl;
    }

    int first_l1out_instruction_num = instruction_count - l1out_neurons;

    for (int j = 0; j < l2out_neurons; j++)
    {
        output_file << instruction_count++ << ",MUL," << first_l1out_instruction_num << std::endl;
        output_file << instruction_count++ << ",ADD," << instruction_count - 2 << std::endl;

        for (int i = 1; i < l1out_neurons; i++)
        {
            output_file << instruction_count++ << ",MUL," << first_l1out_instruction_num + i << std::endl;
            output_file << instruction_count++ << ",ADD," << instruction_count - 3 << "," << instruction_count - 2 << std::endl;
        }
    }
}