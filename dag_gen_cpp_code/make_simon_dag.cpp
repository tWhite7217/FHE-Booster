#include <iostream>
#include <fstream>

const int in_size = 28 * 28;
const int l1out_neurons = 20;
const int l2out_neurons = 10;

const int num_rounds = 2;
const int num_bits = 32;

void set(int[], int[]);
void shift_left(int[], int[], int);
void shift_right(int[], int[], int);
void print_set(int[]);
void print_xor(int[], int[], int[]);
void print_or(int[], int[], int[]);
void print_and(int[], int[], int[]);

std::ofstream output_file;
int instruction_count = 1;

int main()
{
    output_file.open("test_simon.txt");

    output_file << "ADD,1" << std::endl;
    output_file << "MUL,5" << std::endl;
    output_file << "~" << std::endl;

    output_file << instruction_count++ << ",SET" << std::endl;

    int exp_key[num_rounds][num_bits];
    for (int i = 0; i < num_rounds; i++)
    {
        print_set(exp_key[i]);
    }

    int pt0[num_bits];
    print_set(pt0);

    int pt1[num_bits];
    print_set(pt1);

    for (int i = 0; i < num_rounds; i++)
    {
        print_xor(pt0, pt0, pt1);

        int t1mp[num_bits];
        shift_left(t1mp, pt0, 1);

        int t2mp[num_bits];
        shift_right(t2mp, pt0, 15);

        int t3mp[num_bits];
        print_or(t3mp, t1mp, t2mp);

        int t4mp[num_bits];
        shift_left(t4mp, pt0, 8);

        int t5mp[num_bits];
        shift_right(t5mp, pt0, 8);

        int t6mp[num_bits];
        print_or(t6mp, t4mp, t5mp);

        int t7mp[num_bits];
        shift_right(t7mp, pt0, 2);

        int t8mp[num_bits];
        shift_left(t8mp, pt0, 14);

        int t9mp[num_bits];
        print_or(t9mp, t7mp, t8mp);

        int t10mp[num_bits];
        print_and(t10mp, t3mp, t6mp);

        int t11mp[num_bits];
        print_xor(t11mp, t10mp, pt1);

        int t12mp[num_bits];
        print_xor(t12mp, t11mp, t9mp);

        set(pt1, pt0);

        int index = num_rounds - 1 - i;
        print_xor(pt0, t12mp, exp_key[index]);
    }
}

void set(int output_arr[], int input_arr[])
{

    for (int i = 0; i < num_bits; i++)
    {
        output_arr[i] = input_arr[i];
    }
}

void shift_left(int output_arr[], int input_arr[], int num)
{
    for (int i = 0; i < num_bits; i++)
    {
        if (i < num)
        {
            output_arr[i] = 1;
        }
        else
        {
            output_arr[i] = input_arr[i - num];
        }
    }
}

void shift_right(int output_arr[], int input_arr[], int num)
{
    for (int i = 0; i < num_bits; i++)
    {
        if (i + num < num_bits)
        {
            output_arr[i] = input_arr[i + num];
        }
        else
        {
            output_arr[i] = 1;
        }
    }
}

void print_set(int output_arr[])
{
    for (int i = 0; i < num_bits; i++)
    {
        output_file << instruction_count++ << ",SET" << std::endl;
        output_arr[i] = instruction_count - 1;
    }
}

void print_or(int output_arr[], int input_arr1[], int input_arr2[])
{
    for (int i = 0; i < num_bits; i++)
    {
        int c1 = input_arr1[i];
        int c2 = input_arr2[i];
        output_file << instruction_count++ << ",MUL," << c1 << "," << c2 << std::endl;
        output_file << instruction_count++ << ",ADD," << c1 << "," << c2 << std::endl;
        output_file << instruction_count++ << ",SUB," << instruction_count - 3 << "," << instruction_count - 2 << std::endl;
        output_arr[i] = instruction_count - 1;
    }
}

void print_and(int output_arr[], int input_arr1[], int input_arr2[])
{
    for (int i = 0; i < num_bits; i++)
    {
        int c1 = input_arr1[i];
        int c2 = input_arr2[i];
        output_file << instruction_count++ << ",MUL," << c1 << "," << c2 << std::endl;
        output_arr[i] = instruction_count - 1;
    }
}

void print_xor(int output_arr[], int input_arr1[], int input_arr2[])
{
    for (int i = 0; i < num_bits; i++)
    {
        int c1 = input_arr1[i];
        int c2 = input_arr2[i];
        output_file << instruction_count++ << ",SUB," << c1 << "," << c2 << std::endl;
        output_file << instruction_count++ << ",MUL," << instruction_count - 2 << std::endl;
        output_arr[i] = instruction_count - 1;
    }
}