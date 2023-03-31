#include <array>
#include <vector>
#include <iostream>

int main()
{
    const int image_height = 10;
    const int image_width = 10;
    const int num_kernels = 4;
    const std::array<size_t, num_kernels> kernel_sizes = {{3, 5, 7, 3}};
    const int max_kernel_size = *std::max_element(std::begin(kernel_sizes), std::end(kernel_sizes));
    const int padding_each_side = max_kernel_size / 2;
    const int padded_height = image_height + (padding_each_side * 2);
    const int padded_width = image_width + (padding_each_side * 2);

    std::array<std::array<char, image_width>, image_height> input_image = {{{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                                                                            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                                                                            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                                                                            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                                                                            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                                                                            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                                                                            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                                                                            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                                                                            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                                                                            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}};
    std::vector<std::vector<char>> old_image;

    old_image.resize(padded_height);
    for (auto &pixel_row : old_image)
    {
        pixel_row.resize(padded_width);
    }

    std::array<std::array<char, image_width>, image_height> new_image;
    std::array<std::vector<std::vector<char>>, num_kernels> kernels = {{
        {{{0, 0, 0},
          {0, 1, 0},
          {0, 0, 0}}},
        {{{0, 0, 0, 0, 0},
          {0, 0, 0, 0, 0},
          {0, 0, 1, 0, 0},
          {0, 0, 0, 0, 0},
          {0, 0, 0, 0, 0}}},
        {{{0, 0, 0, 0, 0, 0, 0},
          {0, 0, 0, 0, 0, 0, 0},
          {0, 0, 0, -1, 0, 0, 0},
          {0, 0, -1, 5, -1, 0, 0},
          {0, 0, 0, -1, 0, 0, 0},
          {0, 0, 0, 0, 0, 0, 0},
          {0, 0, 0, 0, 0, 0, 0}}},
        {{{0, 0, 0},
          {0, 1, 0},
          {0, 0, 0}}},
    }};

    for (int i = 0; i < padded_height; i++)
    {
        for (int j = 0; j < padding_each_side; j++)
        {
            old_image[i][j] = 0;
            old_image[i][padded_width - j - 1] = 0;
        }
    }

    for (int i = 0; i < padded_width; i++)
    {
        for (int j = 0; j < padding_each_side; j++)
        {
            old_image[j][i] = 0;
            old_image[padded_height - j - 1][i] = 0;
        }
    }

    for (int i = 0; i < image_height; i++)
    {
        for (int j = 0; j < image_width; j++)
        {
            old_image[i + padding_each_side][j + padding_each_side] = input_image[i][j];
        }
    }

    for (int i = 0; i < padded_height; i++)
    {
        for (int j = 0; j < padded_width; j++)
        {
            std::cout << (int)old_image[i][j] << ", ";
        }
        std::cout << std::endl;
    }

    for (int i = 0; i < num_kernels; i++)
    {
        int kernel_size = kernel_sizes[i];
        auto offset = padding_each_side - (kernel_size / 2);
        for (int j = 0; j < image_height; j++)
        {
            for (int k = 0; k < image_width; k++)
            {
                new_image[j][k] = 0;
                for (int q = 0; q < kernel_size; q++)
                {
                    const int old_y = j + q + offset;
                    for (int z = 0; z < kernel_size; z++)
                    {
                        const int old_x = k + z + offset;
                        new_image[j][k] += old_image[old_y][old_x] * kernels[i][q][z];
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

    for (int i = 0; i < image_height; i++)
    {
        for (int j = 0; j < image_width; j++)
        {
            std::cout << (int)new_image[i][j] << ", ";
        }
        std::cout << std::endl;
    }

    return 0;
}