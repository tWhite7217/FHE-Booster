#include "bootstrapping_path_generator.h"

BootstrappingPathGenerator::BootstrappingPathGenerator(OperationList operations, bool using_selective_model)
    : operations{operations}, using_selective_model{using_selective_model} {}

std::vector<std::vector<OperationPtr>> BootstrappingPathGenerator::generate_bootstrapping_paths()
{
    print_number_of_paths();
    create_raw_bootstrapping_paths();
    std::cout << bootstrapping_paths.size() << std::endl;
    // print_bootstrapping_paths();
    clean_raw_bootstrapping_paths();
    return bootstrapping_paths;
}

std::vector<std::vector<OperationPtr>> BootstrappingPathGenerator::generate_bootstrapping_paths_for_validation()
{
    print_number_of_paths();
    create_raw_bootstrapping_paths_for_validation();
    print_bootstrapping_paths();
    clean_raw_bootstrapping_paths();
    return bootstrapping_paths;
}

void BootstrappingPathGenerator::create_raw_bootstrapping_paths()
{
    for (auto operation : operations)
    {
        std::vector<std::vector<OperationPtr>> bootstrapping_paths_to_add;
        if (operation->type == "MUL")
        {
            // bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation, 1, 0);
            // bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation, 1, 0, false);
            // bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation, {}, 1, 0);
            bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation, {}, 1, 0, false);
        }
        else
        {
            // bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation, 0, 1);
            // bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation, 0, 1, true);
            // bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation, {}, 0, 1);
            bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation, {}, 0, 1, true);
        }
        bootstrapping_paths.insert(bootstrapping_paths.end(), bootstrapping_paths_to_add.begin(), bootstrapping_paths_to_add.end());
    }

    class comparator_class
    {
    public:
        bool operator()(std::vector<OperationPtr> path1, std::vector<OperationPtr> path2)
        {
            if (path1.back() == path2.back())
            {
                return path1.size() < path2.size();
            }
            else
            {
                return path1.back()->id < path2.back()->id;
            }
        }
    };

    auto starting_point_offset = 0;
    auto current_starting_id_to_sort = bootstrapping_paths[0][0];
    for (auto i = 1; i < bootstrapping_paths.size(); i++)
    {
        if (bootstrapping_paths[i][0] != current_starting_id_to_sort)
        {
            std::sort(bootstrapping_paths.begin() + starting_point_offset, bootstrapping_paths.begin() + i, comparator_class());
            starting_point_offset = i;
            current_starting_id_to_sort = bootstrapping_paths[i][0];
        }
    }
}

// std::vector<std::vector<OperationPtr>> BootstrappingPathGenerator::create_bootstrapping_paths_helper(OperationPtr operation, int num_multiplications, int num_additions)
// {
//     float path_cost = (num_multiplications + (float)num_additions / addition_divider);
//     if (path_cost > bootstrapping_path_threshold)
//     {
//         return {{operation}};
//     }
//     else if (operation->parent_ptrs.size() == 0)
//     {
//         return {};
//     }

//     std::vector<std::vector<OperationPtr>> paths_to_return;
//     for (auto parent : operation->parent_ptrs)
//     {
//         std::vector<std::vector<OperationPtr>> paths_to_add;
//         if (parent->type == "MUL")
//         {
//             paths_to_add = create_bootstrapping_paths_helper(parent, num_multiplications + 1, num_additions);
//         }
//         else
//         {
//             paths_to_add = create_bootstrapping_paths_helper(parent, num_multiplications, num_additions + 1);
//         }
//         for (auto path : paths_to_add)
//         {
//             path.push_back(operation);
//             paths_to_return.push_back(path);
//         }
//     }
//     return paths_to_return;
// }

// std::vector<std::vector<OperationPtr>> BootstrappingPathGenerator::create_bootstrapping_paths_helper(OperationPtr operation, int num_multiplications, int num_additions, bool initial_was_addition)
// {
//     float path_cost = (num_multiplications + (float)num_additions / addition_divider);
//     if (path_cost > bootstrapping_path_threshold)
//     {
//         float path_cost_without_last_op;
//         if (initial_was_addition)
//         {
//             path_cost_without_last_op = (num_multiplications + (float)(num_additions - 1) / addition_divider);
//         }
//         else
//         {
//             path_cost_without_last_op = ((num_multiplications - 1) + (float)num_additions / addition_divider);
//         }

//         if (path_cost_without_last_op <= bootstrapping_path_threshold)
//         {
//             return {{operation}};
//         }
//         else
//         {
//             return {};
//         }
//     }
//     else if (operation->parent_ptrs.size() == 0)
//     {
//         return {};
//     }

//     std::vector<std::vector<OperationPtr>> paths_to_return;
//     for (auto parent : operation->parent_ptrs)
//     {
//         std::vector<std::vector<OperationPtr>> paths_to_add;
//         if (parent->type == "MUL")
//         {
//             paths_to_add = create_bootstrapping_paths_helper(parent, num_multiplications + 1, num_additions, initial_was_addition);
//         }
//         else
//         {
//             paths_to_add = create_bootstrapping_paths_helper(parent, num_multiplications, num_additions + 1, initial_was_addition);
//         }
//         for (auto path : paths_to_add)
//         {
//             path.push_back(operation);
//             paths_to_return.push_back(path);
//         }
//     }
//     return paths_to_return;
// }

// std::vector<std::vector<OperationPtr>> BootstrappingPathGenerator::create_bootstrapping_paths_helper(OperationPtr operation, std::vector<OperationPtr> path, int num_multiplications, int num_additions)
// {
//     path.push_back(operation);
//     float path_cost = (num_multiplications + (float)num_additions / addition_divider);
//     if (path_cost > bootstrapping_path_threshold)
//     {
//         return {path};
//     }
//     else if (operation->parent_ptrs.size() == 0)
//     {
//         return {};
//     }

//     std::vector<std::vector<OperationPtr>> paths_to_return;
//     for (auto child : operation->child_ptrs)
//     {
//         std::vector<std::vector<OperationPtr>> paths_to_add;
//         if (child->type == "MUL")
//         {
//             paths_to_add = create_bootstrapping_paths_helper(child, path, num_multiplications + 1, num_additions);
//         }
//         else
//         {
//             paths_to_add = create_bootstrapping_paths_helper(child, path, num_multiplications, num_additions + 1);
//         }
//         paths_to_return.insert(paths_to_return.end(), paths_to_add.begin(), paths_to_add.end());
//     }
//     return paths_to_return;
// }

std::vector<std::vector<OperationPtr>> BootstrappingPathGenerator::create_bootstrapping_paths_helper(OperationPtr operation, std::vector<OperationPtr> path, int num_multiplications, int num_additions, bool initial_was_addition)
{
    path.push_back(operation);
    float path_cost = (num_multiplications + (float)num_additions / addition_divider);
    if (path_cost > bootstrapping_path_threshold)
    {
        float path_cost_without_first_op;
        if (initial_was_addition)
        {
            path_cost_without_first_op = (num_multiplications + (float)(num_additions - 1) / addition_divider);
        }
        else
        {
            path_cost_without_first_op = ((num_multiplications - 1) + (float)num_additions / addition_divider);
        }

        if (path_cost_without_first_op <= bootstrapping_path_threshold)
        {
            return {path};
        }
        else
        {
            return {};
        }
    }
    else if (operation->parent_ptrs.size() == 0)
    {
        return {};
    }

    std::vector<std::vector<OperationPtr>> paths_to_return;
    for (auto child : operation->child_ptrs)
    {
        std::vector<std::vector<OperationPtr>> paths_to_add;
        if (child->type == "MUL")
        {
            paths_to_add = create_bootstrapping_paths_helper(child, path, num_multiplications + 1, num_additions, initial_was_addition);
        }
        else
        {
            paths_to_add = create_bootstrapping_paths_helper(child, path, num_multiplications, num_additions + 1, initial_was_addition);
        }
        paths_to_return.insert(paths_to_return.end(), paths_to_add.begin(), paths_to_add.end());
    }
    return paths_to_return;
}

void BootstrappingPathGenerator::create_raw_bootstrapping_paths_for_validation()
{
    std::vector<std::vector<OperationPtr>> all_backward_paths;
    for (auto operation : operations)
    {
        auto backward_paths_from_operation = depth_first_search(
            {operation}, {});
        all_backward_paths.insert(all_backward_paths.end(), backward_paths_from_operation.begin(), backward_paths_from_operation.end());
    }

    std::vector<std::vector<OperationPtr>> paths_above_threshold;
    for (auto &path : all_backward_paths)
    {
        if (get_path_cost(path) > bootstrapping_path_threshold)
        {
            std::reverse(path.begin(), path.end());
            paths_above_threshold.push_back(path);
        }
    }

    auto i = 0;
    for (auto path : paths_above_threshold)
    {
        path.pop_back();
        if (get_path_cost(path) <= bootstrapping_path_threshold)
        {
            bootstrapping_paths.push_back(paths_above_threshold[i]);
        }
        i++;
    }
}

std::vector<std::vector<OperationPtr>> BootstrappingPathGenerator::depth_first_search(std::vector<OperationPtr> current_path, std::vector<std::vector<OperationPtr>> backward_paths)
{
    auto current_end_of_path = current_path.back();
    if (current_end_of_path->parent_ptrs.size() > 0)
    {
        for (auto parent : current_end_of_path->parent_ptrs)
        {
            std::vector<OperationPtr> new_path;
            std::copy(current_path.begin(), current_path.end(), std::back_inserter(new_path));
            new_path.push_back(parent);
            backward_paths = depth_first_search(new_path, backward_paths);
        }
    }
    backward_paths.push_back(current_path);
    return backward_paths;
}

void BootstrappingPathGenerator::clean_raw_bootstrapping_paths()
{
    remove_redundant_bootstrapping_paths();

    if (!using_selective_model)
    {
        remove_last_operation_from_bootstrapping_paths();
        std::cout << "here2" << std::endl;
        remove_duplicate_bootstrapping_paths();
        std::cout << "here3" << std::endl;
    }

    // remove_redundant_bootstrapping_paths();
}

void BootstrappingPathGenerator::remove_last_operation_from_bootstrapping_paths()
{
    for (auto &path : bootstrapping_paths)
    {
        path.pop_back();
    }
}

void BootstrappingPathGenerator::remove_duplicate_bootstrapping_paths()
{
    if (bootstrapping_paths.size() <= 1)
    {
        return;
    }

    std::sort(bootstrapping_paths.begin(), bootstrapping_paths.end());

    auto logical_end = std::unique(bootstrapping_paths.begin(), bootstrapping_paths.end());

    bootstrapping_paths.erase(logical_end,
                              bootstrapping_paths.end());
}

void BootstrappingPathGenerator::remove_redundant_bootstrapping_paths()
{
    // std::vector<size_t> indices_to_remove;
    for (auto i = 0; i < bootstrapping_paths.size(); i++)
    {
        for (auto j = i + 1;
             j < bootstrapping_paths.size() &&
             bootstrapping_paths[i].front() == bootstrapping_paths[j].front() &&
             bootstrapping_paths[i].back() == bootstrapping_paths[j].back();
             j++)
        {
            if (bootstrapping_paths[i].size() != bootstrapping_paths[j].size())
            {
                if (paths_are_redundant(bootstrapping_paths[i], bootstrapping_paths[j]))
                {
                    bootstrapping_paths.erase(bootstrapping_paths.begin() + j);
                    j--;
                }
            }
        }
    }

    // int num_redundant_paths = indices_to_remove.size();
    // for (int i = num_redundant_paths - 1; i >= 0; i--)
    // {
    //     bootstrapping_paths.erase(bootstrapping_paths.begin() + indices_to_remove[i]);
    // }
}

bool BootstrappingPathGenerator::paths_are_redundant(std::vector<OperationPtr> path1, std::vector<OperationPtr> path2)
{
    auto j = 0;
    for (auto i = 0; i < path1.size(); i++)
    {
        while (path1[i] != path2[j])
        {
            j++;
            if (j >= path2.size())
            {
                return false;
            }
        }
    }

    for (auto operation : path1)
    {
        std::cout << operation->id << ",";
    }
    std::cout << std::endl;

    for (auto operation : path2)
    {
        std::cout << operation->id << ",";
    }
    std::cout << std::endl;

    return true;
}

// void BootstrappingPathGenerator::remove_redundant_bootstrapping_paths()
// {
//     size_t min_path_length = std::numeric_limits<size_t>::max();
//     for (auto path : bootstrapping_paths)
//     {
//         min_path_length = std::min(min_path_length, path.size());
//     }

//     std::vector<size_t> indices_to_remove;
//     for (auto i = 0; i < bootstrapping_paths.size(); i++)
//     {
//         if (bootstrapping_paths[i].size() > min_path_length)
//         {
//             if (path_is_redundant(i))
//             {
//                 indices_to_remove.push_back(i);
//             }
//         }
//     }

//     int num_redundant_paths = indices_to_remove.size();
//     for (int i = num_redundant_paths - 1; i >= 0; i--)
//     {
//         bootstrapping_paths.erase(bootstrapping_paths.begin() + indices_to_remove[i]);
//     }
// }

bool BootstrappingPathGenerator::path_is_redundant(size_t path_index)
{
    auto test_path = bootstrapping_paths[path_index];
    auto test_path_size = test_path.size();
    for (auto i = 0; i < bootstrapping_paths.size(); i++)
    {
        auto path_i = bootstrapping_paths[i];
        auto path_i_size = path_i.size();
        auto size_difference = test_path_size - path_i_size;
        if (i != path_index && size_difference > 0)
        {
            if (larger_path_contains_smaller_path(test_path, path_i))
            {
                return true;
            }
        }
    }
    return false;
}

bool BootstrappingPathGenerator::larger_path_contains_smaller_path(std::vector<OperationPtr> larger_path, std::vector<OperationPtr> smaller_path)
{
    if (using_selective_model)
    {
        larger_path.pop_back();
    }

    for (auto i = 0; i < smaller_path.size(); i++)
    {
        if (!vector_contains_element(larger_path, smaller_path[i]))
        {
            return false;
        }
    }

    for (auto operation : smaller_path)
    {
        std::cout << operation->id << ",";
    }
    std::cout << std::endl;

    for (auto operation : larger_path)
    {
        std::cout << operation->id << ",";
    }
    std::cout << std::endl;

    return true;
}

void BootstrappingPathGenerator::print_number_of_paths()
{
    std::unordered_map<OperationPtr, int> num_paths_to;
    for (auto operation : operations)
    {
        num_paths_to[operation] = 1;
        for (auto parent : operation->parent_ptrs)
        {
            num_paths_to[operation] += num_paths_to[parent] + 1;
        }
    }

    int result = std::accumulate(std::begin(num_paths_to), std::end(num_paths_to), 0,
                                 [](const int previous, const std::pair<const OperationPtr, int> &p)
                                 { return previous + p.second; });

    std::cout << result << std::endl;
}

void BootstrappingPathGenerator::print_bootstrapping_paths()
{
    for (auto path : bootstrapping_paths)
    {
        for (auto operation : path)
        {
            std::cout << operation->id << ",";
        }
        std::cout << std::endl;
    }
}