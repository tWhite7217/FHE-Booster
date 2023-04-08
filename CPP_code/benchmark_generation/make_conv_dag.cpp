#include "dag_generator.h"

const int image_height = 5;
const int image_width = 5;
const int kernel_size = 3;
const int padding_each_side = kernel_size / 2;
const int padded_height = image_height + (padding_each_side * 2);
const int padded_width = image_width + (padding_each_side * 2);
const int num_kernels = 32;

int main(int argc, char *argv[])
{
    std::ofstream output_file(argv[1]);

    auto dg = DAGGenerator(output_file);

    dg.set_zero_var();

    std::array<std::array<variable, image_width>, image_height> input_image;
    std::array<std::array<std::array<variable, kernel_size>, kernel_size>, num_kernels> kernels;

    for (auto &pixel_row : input_image)
    {
        dg.print_set_array(pixel_row);
    }
    for (auto &kernel : kernels)
    {
        for (auto &kernel_row : kernel)
        {
            dg.print_set_array(kernel_row);
        }
    }

    output_file << "~" << std::endl;

    std::array<std::array<variable, padded_width>, padded_height> old_image;
    std::array<std::array<variable, image_width>, image_height> new_image;

    // std::array<std::array<variable, kernel_size>, kernel_size> tmps;

    for (int i = 0; i < padded_height; i++)
    {
        for (int j = 0; j < padding_each_side; j++)
        {
            old_image[i][j] = dg.zero_var;
            old_image[i][padded_width - j - 1] = dg.zero_var;
        }
    }

    for (int i = 0; i < padded_width; i++)
    {
        for (int j = 0; j < padding_each_side; j++)
        {
            old_image[j][i] = dg.zero_var;
            old_image[padded_height - j - 1][i] = dg.zero_var;
        }
    }

    for (int i = 0; i < image_height; i++)
    {
        for (int j = 0; j < image_width; j++)
        {
            old_image[i + padding_each_side][j + padding_each_side] = input_image[i][j];
        }
    }

    variable tmp;

    for (int i = 0; i < num_kernels; i++)
    {
        for (int j = 0; j < image_height; j++)
        {
            for (int k = 0; k < image_width; k++)
            {
                // new_image[j][k] = 0;
                for (int q = 0; q < kernel_size; q++)
                {
                    const int old_y = j + q;
                    for (int z = 0; z < kernel_size; z++)
                    {
                        const int old_x = k + z;
                        dg.print_mul(tmp, old_image[old_y][old_x], kernels[i][q][z]);
                        if (q == 0 && z == 0)
                        {
                            new_image[j][k] = tmp;
                        }
                        dg.print_add(new_image[j][k], new_image[j][k], tmp);
                    }
                }
            }
        }

        for (int j = 0; j < image_height; j++)
        {
            for (int k = 0; k < image_width; k++)
            {
                old_image[j + padding_each_side][k + padding_each_side] = new_image[j][k];
            }
        }
    }
}