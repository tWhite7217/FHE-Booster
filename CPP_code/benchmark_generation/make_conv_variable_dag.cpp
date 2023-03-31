#include "dag_generator.h"

const int image_height = 8;
const int image_width = 8;
const int num_kernels = 2;
// const int num_kernels = 54;
const std::array<size_t, num_kernels> kernel_sizes = {{3, 5}};
const int max_kernel_size = *std::max_element(std::begin(kernel_sizes), std::end(kernel_sizes));
const int padding_each_side = max_kernel_size / 2;
const int padded_height = image_height + (padding_each_side * 2);
const int padded_width = image_width + (padding_each_side * 2);

int main(int argc, char *argv[])
{
    std::ofstream output_file(argv[1]);

    auto dg = DAGGenerator(output_file);

    dg.set_zero_var();

    std::array<std::array<variable, image_width>, image_height> input_image;

    std::array<std::vector<std::vector<variable>>, num_kernels> kernels;
    for (auto i = 0; i < kernels.size(); i++)
    {
        auto kernel_size = kernel_sizes[i];
        auto &kernel = kernels[i];
        kernel.resize(kernel_size);
        for (auto &kernel_row : kernel)
        {
            kernel_row.resize(kernel_size);
        }
    }

    for (auto &pixel_row : input_image)
    {
        dg.print_set_array(pixel_row);
    }
    for (auto &kernel : kernels)
    {
        for (auto &kernel_row : kernel)
        {
            dg.print_set_vector(kernel_row);
        }
    }

    output_file << "~" << std::endl;

    std::vector<std::vector<variable>> old_image;
    old_image.resize(padded_height);
    for (auto &pixel_row : old_image)
    {
        pixel_row.resize(padded_width);
    }

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
        int kernel_size = kernel_sizes[i];
        auto offset = padding_each_side - (kernel_size / 2);
        for (int j = 0; j < image_height; j++)
        {
            for (int k = 0; k < image_width; k++)
            {
                // new_image[j][k] = 0;
                for (int q = 0; q < kernel_size; q++)
                {
                    const int old_y = j + q + offset;
                    for (int z = 0; z < kernel_size; z++)
                    {
                        const int old_x = k + z + offset;
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