#include "dag_generator.h"

const int num_rounds = 32;
const int num_bits = 16;

int main(int argc, char *argv[])
{
    std::ofstream output_file(argv[1]);

    auto dg = DAGGenerator(output_file);

    dg.set_zero_bit();

    std::array<std::array<bit, num_bits>, num_rounds> exp_key;
    for (int i = 0; i < num_rounds; i++)
    {
        dg.print_set(exp_key[i]);
    }

    std::array<bit, num_bits> pt0;
    dg.print_set(pt0);

    std::array<bit, num_bits> pt1;
    dg.print_set(pt1);

    output_file << "~" << std::endl;

    for (int i = 0; i < num_rounds; i++)
    {
        // print_xor(pt0, pt0, pt1);

        std::array<bit, num_bits> t1mp;
        dg.print_rotate_left(t1mp, pt0, 1);

        std::array<bit, num_bits> t2mp;
        dg.print_rotate_left(t2mp, pt0, 8);

        std::array<bit, num_bits> t3mp;
        dg.print_rotate_left(t3mp, pt0, 2);

        std::array<bit, num_bits> t4mp;
        dg.print_and(t4mp, t1mp, t2mp);

        std::array<bit, num_bits> t5mp;
        int index = num_rounds - 1 - i;
        dg.print_xor(t5mp, pt1, exp_key[index]);

        std::array<bit, num_bits> t6mp;
        dg.print_xor(t6mp, t3mp, t5mp);

        dg.set(pt1, pt0);

        dg.print_xor(pt0, t4mp, t6mp);
    }
}