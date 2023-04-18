#include <array>
#include <iostream>

int main()
{
    const int image_height = 5;
    const int image_width = 5;
    const int kernel_size = 3;
    const int padding_each_side = kernel_size - 2;
    const int padded_height = image_height + (padding_each_side * 2);
    const int padded_width = image_width + (padding_each_side * 2);
    const int num_kernels = 32;

    std::array<std::array<char, image_width>, image_height>
        input_image =
            {{{1, 1, 1, 1, 1},
              {1, 1, 1, 1, 1},
              {1, 1, 1, 1, 1},
              {1, 1, 1, 1, 1},
              {1, 1, 1, 1, 1}}};

    std::array<
        std::array<std::array<char, kernel_size>, kernel_size>,
        num_kernels>
        kernels;

    kernels.fill({{{0, 0, 0},
                   {0, 1, 0},
                   {0, 0, 0}}});

    std::array<std::array<char, padded_width>, padded_height>
        old_image;
    std::array<std::array<char, image_width>, image_height>
        new_image;

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
            const int y = i + padding_each_side;
            const int x = j + padding_each_side;
            old_image[y][x] = input_image[i][j];
        }
    }

    for (int i = 0; i < num_kernels; i++)
    {
        for (int j = 0; j < image_height; j++)
        {
            for (int k = 0; k < image_width; k++)
            {
                new_image[j][k] = 0;
                for (int q = 0; q < kernel_size; q++)
                {
                    const int old_y = j + q;
                    for (int z = 0; z < kernel_size; z++)
                    {
                        const int old_x = k + z;
                        new_image[j][k] +=
                            old_image[old_y][old_x] *
                            kernels[i][q][z];
                    }
                }
            }
        }

        for (int j = 0; j < image_height; j++)
        {
            for (int k = 0; k < image_width; k++)
            {
                const int y = j + padding_each_side;
                const int x = k + padding_each_side;
                old_image[y][x] = new_image[j][k];
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