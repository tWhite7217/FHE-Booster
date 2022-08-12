#include "bootstrapping_path_generator.h"

BootstrappingPathGenerator::BootstrappingPathGenerator(OperationList operations, bool using_selective_model)
    : operations{operations}, using_selective_model{using_selective_model} {}

std::vector<std::vector<OperationPtr>> BootstrappingPathGenerator::generate_bootstrapping_paths()
{
    create_raw_bootstrapping_paths();
    clean_raw_bootstrapping_paths();
    return bootstrapping_paths;
}

std::vector<std::vector<OperationPtr>> BootstrappingPathGenerator::generate_bootstrapping_paths_for_validation()
{
    create_raw_bootstrapping_paths_for_validation();
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
            bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation, 1, 0);
        }
        else
        {
            bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation, 0, 1);
        }
        bootstrapping_paths.insert(bootstrapping_paths.end(), bootstrapping_paths_to_add.begin(), bootstrapping_paths_to_add.end());
    }
}

std::vector<std::vector<OperationPtr>> BootstrappingPathGenerator::create_bootstrapping_paths_helper(OperationPtr operation, int num_multiplications, int num_additions)
{
    float path_cost = (num_multiplications + (float)num_additions / addition_divider);
    if (path_cost > bootstrapping_path_threshold)
    {
        return {{operation}};
    }
    else if (operation->parent_ptrs.size() == 0)
    {
        return {};
    }

    std::vector<std::vector<OperationPtr>> paths_to_return;
    for (auto parent : operation->parent_ptrs)
    {
        std::vector<std::vector<OperationPtr>> paths_to_add;
        if (parent->type == "MUL")
        {
            paths_to_add = create_bootstrapping_paths_helper(parent, num_multiplications + 1, num_additions);
        }
        else
        {
            paths_to_add = create_bootstrapping_paths_helper(parent, num_multiplications, num_additions + 1);
        }
        for (auto path : paths_to_add)
        {
            path.push_back(operation);
            paths_to_return.push_back(path);
        }
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

float BootstrappingPathGenerator::get_path_cost(std::vector<OperationPtr> path)
{
    int num_multiplications = 0;
    int num_additions = 0;
    for (auto operation : path)
    {
        if (operation->type == "MUL")
        {
            num_multiplications++;
        }
        else
        {
            num_additions++;
        }
    }
    return num_multiplications + (float)num_additions / addition_divider;
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
    if (!using_selective_model)
    {
        remove_last_operation_from_bootstrapping_paths();
        remove_duplicate_bootstrapping_paths();
    }

    remove_redundant_bootstrapping_paths();
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
    size_t min_path_length = std::numeric_limits<size_t>::max();
    for (auto path : bootstrapping_paths)
    {
        min_path_length = std::min(min_path_length, path.size());
    }

    std::vector<size_t> indices_to_remove;
    for (auto i = 0; i < bootstrapping_paths.size(); i++)
    {
        if (bootstrapping_paths[i].size() > min_path_length)
        {
            if (path_is_redundant(i))
            {
                indices_to_remove.push_back(i);
            }
        }
    }

    int num_redundant_paths = indices_to_remove.size();
    for (int i = num_redundant_paths - 1; i >= 0; i--)
    {
        bootstrapping_paths.erase(bootstrapping_paths.begin() + indices_to_remove[i]);
    }
}

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
    
    return true;
}
