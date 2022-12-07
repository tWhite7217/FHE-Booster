#include <iostream>
#include <fstream>
#include <string>

const int in_size = 28 * 28;
const int l1out_neurons = 20;
const int l2out_neurons = 10;

const int num_rounds = 10;
const int num_bits = 16;

using bit = std::string;

void set(bit[], bit[]);
void print_rotate_left(bit[], bit[], int);
void shift_left(bit[], bit[], int);
void shift_right(bit[], bit[], int);
void print_set(bit[]);
void print_xor(bit[], bit[], bit[]);
void print_or(bit[], bit[], bit[]);
void print_and(bit[], bit[], bit[]);

std::ofstream output_file;
int instruction_count = 1;
int constant_count = 1;

int main()
{
    auto output_filename = "test_simon_" + std::to_string(num_rounds) + "_rounds.txt";
    output_file.open(output_filename);

    output_file << "ADD,1" << std::endl;
    output_file << "SUB,1" << std::endl;
    output_file << "MUL,5" << std::endl;
    output_file << "~" << std::endl;

    output_file << constant_count++ << ",SET" << std::endl;

    bit exp_key[num_rounds][num_bits];
    for (int i = 0; i < num_rounds; i++)
    {
        print_set(exp_key[i]);
    }

    bit pt0[num_bits];
    print_set(pt0);

    bit pt1[num_bits];
    print_set(pt1);

    output_file << "~" << std::endl;

    for (int i = 0; i < num_rounds; i++)
    {
        // print_xor(pt0, pt0, pt1);

        bit t1mp[num_bits];
        print_rotate_left(t1mp, pt0, 1);

        bit t2mp[num_bits];
        print_rotate_left(t2mp, pt0, 8);

        bit t3mp[num_bits];
        print_rotate_left(t3mp, pt0, 2);

        bit t4mp[num_bits];
        print_and(t4mp, t1mp, t2mp);

        bit t5mp[num_bits];
        int index = num_rounds - 1 - i;
        print_xor(t5mp, pt1, exp_key[index]);

        bit t6mp[num_bits];
        print_xor(t6mp, t3mp, t5mp);

        set(pt1, pt0);

        print_xor(pt0, t4mp, t6mp);
    }
}

void set(bit output_arr[], bit input_arr[])
{
    for (int i = 0; i < num_bits; i++)
    {
        output_arr[i] = input_arr[i];
    }
}

void print_rotate_left(bit output_arr[], bit input_arr[], int num)
{
    bit t1mp[num_bits];
    shift_left(t1mp, input_arr, num);

    bit t2mp[num_bits];
    shift_right(t2mp, input_arr, num_bits - num);

    bit t3mp[num_bits];
    print_or(output_arr, t1mp, t2mp);
}

// void print_rotate_right(bit output_arr[], bit input_arr[], int num)
// {
//     bit t1mp[num_bits];
//     shift_right(t1mp, input_arr, num);

//     bit t2mp[num_bits];
//     shift_left(t2mp, input_arr, num_bits - num);

//     bit t3mp[num_bits];
//     print_or(output_arr, t1mp, t2mp);
// }

void shift_left(bit output_arr[], bit input_arr[], int num)
{
    for (int i = 0; i < num_bits; i++)
    {
        if (i < num)
        {
            output_arr[i] = "k1";
        }
        else
        {
            output_arr[i] = input_arr[i - num];
        }
    }
}

void shift_right(bit output_arr[], bit input_arr[], int num)
{
    for (int i = 0; i < num_bits; i++)
    {
        if (i + num < num_bits)
        {
            output_arr[i] = input_arr[i + num];
        }
        else
        {
            output_arr[i] = "k1";
        }
    }
}

void print_set(bit output_arr[])
{
    for (int i = 0; i < num_bits; i++)
    {
        output_file << constant_count++ << ",SET" << std::endl;
        output_arr[i] = "k" + std::to_string(constant_count - 1);
    }
}

void print_or(bit output_arr[], bit input_arr1[], bit input_arr2[])
{
    for (int i = 0; i < num_bits; i++)
    {
        bit c1 = input_arr1[i];
        bit c2 = input_arr2[i];
        output_file << instruction_count++ << ",MUL," << c1 << "," << c2 << std::endl;
        output_file << instruction_count++ << ",ADD," << c1 << "," << c2 << std::endl;
        output_file << instruction_count++ << ",SUB,c" << instruction_count - 3 << ",c" << instruction_count - 2 << std::endl;
        output_arr[i] = "c" + std::to_string(instruction_count - 1);
    }
}

void print_and(bit output_arr[], bit input_arr1[], bit input_arr2[])
{
    for (int i = 0; i < num_bits; i++)
    {
        bit c1 = input_arr1[i];
        bit c2 = input_arr2[i];
        output_file << instruction_count++ << ",MUL," << c1 << "," << c2 << std::endl;
        output_arr[i] = "c" + std::to_string(instruction_count - 1);
    }
}

void print_xor(bit output_arr[], bit input_arr1[], bit input_arr2[])
{
    for (int i = 0; i < num_bits; i++)
    {
        bit c1 = input_arr1[i];
        bit c2 = input_arr2[i];
        output_file << instruction_count++ << ",SUB," << c1 << "," << c2 << std::endl;
        output_file << instruction_count++ << ",MUL,c" << instruction_count - 2 << std::endl;
        output_arr[i] = "c" + std::to_string(instruction_count - 1);
    }
}