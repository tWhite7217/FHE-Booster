#include "shared_utils.h"

void remove_chars_from_string(std::string &str, std::vector<char> chars_to_remove)
{
    for (unsigned int i = 0; i < chars_to_remove.size(); i++)
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
    return find_unsatisfied_bootstrapping_path_index(bootstrapping_paths, bootstrapping_path_is_satisfied) == -1;
}

bool bootstrapping_paths_are_satisfied_for_selective_model(std::vector<OperationList> &bootstrapping_paths)
{
    return find_unsatisfied_bootstrapping_path_index(bootstrapping_paths, bootstrapping_path_is_satisfied_for_selective_model) == -1;
}

int find_unsatisfied_bootstrapping_path_index(std::vector<OperationList> &bootstrapping_paths, std::function<bool(OperationList &)> path_is_satisfied)
{
    for (int i = 0; i < bootstrapping_paths.size(); i++)
    {
        if (!path_is_satisfied(bootstrapping_paths[i]))
        {
            return i;
        }
    }
    return -1;
}

bool bootstrapping_path_is_satisfied(OperationList &bootstrapping_path)
{
    for (auto operation : bootstrapping_path)
    {
        if (operation_is_bootstrapped(operation))
        {
            return true;
        }
    }
    return false;
}


bool bootstrapping_path_is_satisfied_for_selective_model(OperationList &bootstrapping_path)
{
    for (auto i = 0; i < bootstrapping_path.size() - 1; i++)
    {
        if (vector_contains_element(bootstrapping_path[i]->child_ptrs_that_receive_bootstrapped_result, bootstrapping_path[i + 1]))
        {
            return true;
        }
    }
    return false;
}

bool operation_is_bootstrapped(OperationPtr operation)
{
    return operation->child_ptrs_that_receive_bootstrapped_result.size() > 0;
}

float get_path_cost(std::vector<OperationPtr> path)
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