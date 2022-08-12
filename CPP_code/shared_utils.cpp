#include "shared_utils.h"

void remove_chars_from_string(std::string &str, std::vector<char> chars_to_remove)
{
    for (unsigned int i = 0; i < chars_to_remove.size(); ++i)
    {
        str.erase(remove(str.begin(), str.end(), chars_to_remove[i]), str.end());
    }
}

int extract_number_from_string(std::string str, size_t start_index, size_t end_index)
{
    auto num_digits = end_index - start_index;
    std::string num_as_string = str.substr(start_index, num_digits);
    int num = std::stoi(num_as_string);
    return num;
}

bool operations_bootstrap_on_same_core(OperationPtr op1, OperationPtr op2)
{
    return op1->core_num == op2->core_num;
}

OperationPtr get_operation_ptr_from_id(OperationList operations, int id)
{
    if (id < 1 || id > operations.size())
    {
        throw std::runtime_error("Invalid operation id");
    }
    return operations[id - 1];
}

void add_child_ptrs_to_operation_list_with_existing_parent_ptrs(OperationList operations)
{
    for (auto operation : operations)
    {
        for (auto parent : operation->parent_ptrs)
        {
            parent->child_ptrs.push_back(operation);
        }
    }
}

bool bootstrapping_paths_are_satisfied(std::vector<OperationList> &bootstrapping_paths)
{
    return find_unsatisfied_bootstrapping_path_index(bootstrapping_paths) == -1;
}

bool bootstrapping_paths_are_satisfied_for_selective_model(std::vector<OperationList> &bootstrapping_paths)
{
    return find_unsatisfied_bootstrapping_path_index_for_selective_model(bootstrapping_paths) == -1;
}

int find_unsatisfied_bootstrapping_path_index(std::vector<OperationList> &bootstrapping_paths)
{
    for (auto i = 0; i < bootstrapping_paths.size(); ++i)
    {
        auto path = bootstrapping_paths[i];
        bool path_satisfied = false;
        for (auto operation : path)
        {
            if (operation_is_bootstrapped(operation))
            {
                path_satisfied = true;
                break;
            }
        }

        if (!path_satisfied)
        {
            return i;
        }
    }
    return -1;
}

int find_unsatisfied_bootstrapping_path_index_for_selective_model(std::vector<OperationList> &bootstrapping_paths)
{
    for (auto i = 0; i < bootstrapping_paths.size(); ++i)
    {
        auto path = bootstrapping_paths[i];
        bool path_satisfied = false;
        for (auto j = 0; j < path.size() - 1; j++)
        {
            if (vector_contains_element(path[j]->child_ptrs_that_receive_bootstrapped_result, path[j + 1]))
            {
                path_satisfied = true;
                break;
            }
        }

        if (!path_satisfied)
        {
            return i;
        }
    }
    return -1;
}

bool operation_is_bootstrapped(OperationPtr operation)
{
    return operation->child_ptrs_that_receive_bootstrapped_result.size() > 0;
}