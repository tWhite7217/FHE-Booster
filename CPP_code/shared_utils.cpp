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

OperationPtr get_operation_ptr_from_id(OperationList &operations, int id)
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

std::vector<std::string> split_string_by_character(std::string str, char separator)
{
    std::vector<std::string> str_as_list;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, separator))
    {
        str_as_list.push_back(item);
    }
    return str_as_list;
}

bool path_is_urgent(OperationList &path)
{
    auto first_operation = path.front();
    return !bootstrapping_path_is_satisfied(path) && operation_parents_meet_urgency_criteria(first_operation);
}

bool operation_parents_meet_urgency_criteria(OperationPtr &operation)
{
    for (auto parent : operation->parent_ptrs)
    {
        if ((parent->path_nums.size() > 0) && !operation_is_bootstrapped(parent))
        {
            return false;
        }
    }
    return true;
}

bool operation_has_no_parents(OperationPtr &operation)
{
    return (operation->parent_ptrs.size() == 0) &&
           (operation->constant_parent_ids.size() == 0);
}

bool operation_receives_a_bootstrapped_result_from_parent(const OperationPtr &operation, const OperationPtr &parent)
{
    return vector_contains_element(parent->child_ptrs_that_receive_bootstrapped_result, operation);
}

bool operation_has_multiplication_child(const OperationPtr &operation)
{
    for (const auto &child : operation->child_ptrs)
    {
        if (child->type == "MUL")
        {
            return true;
        }
    }
    return false;
}

bool arg_exists(const std::string options_string, const std::string &short_form, const std::string &long_form)
{
    bool short_form_exists = options_string.find(" " + short_form + " ") != std::string::npos;
    bool long_form_exists = options_string.find(" " + long_form + " ") != std::string::npos;
    return short_form_exists || long_form_exists;
}

std::string get_arg(const std::string &options_string, const std::string &short_form, const std::string &long_form, const std::string &help_info)
{
    auto short_pos = options_string.find(short_form);
    auto long_pos = options_string.find(long_form);
    size_t start_pos;
    char expected_char;
    if (short_pos != std::string::npos)
    {
        start_pos = short_pos + short_form.size() + 1;
        expected_char = ' ';
    }
    else if (long_pos != std::string::npos)
    {
        start_pos = long_pos + long_form.size() + 1;
        expected_char = '=';
    }
    else
    {
        return "";
    }

    if (options_string.at(start_pos - 1) != expected_char)
    {
        std::cout << "Options must follow the format shown." << std::endl;
        std::cout << help_info << std::endl;
        exit(-1);
    }

    auto end_pos = options_string.substr(start_pos, options_string.size() - start_pos).find(" ");
    return options_string.substr(start_pos, end_pos);
}

bool bool_arg_converter(const std::string &arg_val)
{
    if (arg_val == "y")
    {
        return true;
    }
    else if (arg_val == "n")
    {
        return false;
    }

    throw;
}

void print_size_mismatch_error(const size_t &expected_size, const size_t &actual_size, const std::string &short_form, const std::string &long_form)
{
    std::cout << "Command line argument " << short_form << "/" << long_form << " has " << actual_size << "elements, but was expected to have " << expected_size << " elements." << std::endl;
    exit(1);
}