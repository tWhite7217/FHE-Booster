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

bool bootstrapping_segments_are_satisfied(std::vector<OperationList> &bootstrapping_segments)
{
    return find_unsatisfied_bootstrapping_segment_index(bootstrapping_segments, bootstrapping_segment_is_satisfied) == -1;
}

bool bootstrapping_segments_are_satisfied_for_selective_model(std::vector<OperationList> &bootstrapping_segments)
{
    return find_unsatisfied_bootstrapping_segment_index(bootstrapping_segments, bootstrapping_segment_is_satisfied_for_selective_model) == -1;
}

int find_unsatisfied_bootstrapping_segment_index(std::vector<OperationList> &bootstrapping_segments, std::function<bool(OperationList &)> segment_is_satisfied)
{
    for (int i = 0; i < bootstrapping_segments.size(); i++)
    {
        if (!segment_is_satisfied(bootstrapping_segments[i]))
        {
            return i;
        }
    }
    return -1;
}

bool bootstrapping_segment_is_satisfied(OperationList &bootstrapping_segment)
{
    for (auto operation : bootstrapping_segment)
    {
        if (operation_is_bootstrapped(operation))
        {
            return true;
        }
    }
    return false;
}

bool bootstrapping_segment_is_satisfied_for_selective_model(OperationList &bootstrapping_segment)
{
    for (auto i = 0; i < bootstrapping_segment.size() - 1; i++)
    {
        if (vector_contains_element(bootstrapping_segment[i]->child_ptrs_that_receive_bootstrapped_result, bootstrapping_segment[i + 1]))
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

bool segment_is_urgent(OperationList &segment)
{
    auto first_operation = segment.front();
    return !bootstrapping_segment_is_satisfied(segment) && operation_parents_meet_urgency_criteria(first_operation);
}

bool operation_parents_meet_urgency_criteria(OperationPtr &operation)
{
    for (auto parent : operation->parent_ptrs)
    {
        if ((parent->segment_nums.size() > 0) && !operation_is_bootstrapped(parent))
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

void update_earliest_start_time(OperationPtr &operation, const std::map<std::string, int> &operation_type_to_latency_map)
{
    for (auto parent : operation->parent_ptrs)
    {
        int parent_latency = operation_type_to_latency_map.at(parent->type);
        if (vector_contains_element(parent->child_ptrs_that_receive_bootstrapped_result, operation))
        {
            parent_latency += bootstrapping_latency;
        }
        int possible_est =
            parent->earliest_start_time +
            parent_latency;
        if (possible_est > operation->earliest_start_time)
        {
            operation->earliest_start_time = possible_est;
        }
    }
}

int get_earliest_possible_program_end_time(OperationList &operations, const std::map<std::string, int> &operation_type_to_latency_map)
{
    int earliest_possible_program_end_time = 0;
    for (auto operation : operations)
    {
        auto latency = operation_type_to_latency_map.at(operation->type);
        int operation_earliest_end_time = operation->earliest_start_time + latency;
        if (operation_earliest_end_time > earliest_possible_program_end_time)
        {
            earliest_possible_program_end_time = operation_earliest_end_time;
        }
    }
    return earliest_possible_program_end_time;
}

void update_latest_start_time(OperationPtr &operation, int earliest_possible_program_end_time, const std::map<std::string, int> &operation_type_to_latency_map)
{
    operation->latest_start_time = std::numeric_limits<decltype(operation->latest_start_time)>::max();

    if (operation->child_ptrs.empty())
    {
        operation->latest_start_time =
            earliest_possible_program_end_time - operation_type_to_latency_map.at(operation->type);
    }
    else
    {
        for (auto child : operation->child_ptrs)
        {
            int child_latency = operation_type_to_latency_map.at(child->type);
            if (operation_is_bootstrapped(child))
            {
                child_latency += bootstrapping_latency;
            }
            int possible_lst =
                child->latest_start_time -
                child_latency;
            if (possible_lst < operation->latest_start_time)
            {
                operation->latest_start_time = possible_lst;
            }
        }
    }
}

void update_all_ESTs_and_LSTs(OperationList &operations, const std::map<std::string, int> &operation_type_to_latency_map)
{
    // auto operations_in_topological_order = get_operations_in_topological_order();
    // TGFF puts operations in topological order so we do not need to do that here
    auto operations_in_topological_order = operations;

    for (auto operation : operations_in_topological_order)
    {
        update_earliest_start_time(operation, operation_type_to_latency_map);
    }

    int earliest_program_end_time = get_earliest_possible_program_end_time(operations, operation_type_to_latency_map);

    auto operations_in_reverse_topological_order = operations_in_topological_order;
    std::reverse(operations_in_reverse_topological_order.begin(), operations_in_reverse_topological_order.end());

    for (auto operation : operations_in_reverse_topological_order)
    {
        update_latest_start_time(operation, earliest_program_end_time, operation_type_to_latency_map);
    }
}

int update_all_slacks(OperationList &operations)
{
    int max = 0;
    for (auto &operation : operations)
    {
        int slack = operation->latest_start_time - operation->earliest_start_time;
        operation->slack = slack;
        if (slack > max)
        {
            max = slack;
        }
    }
    return max;
}

void add_segment_num_info_to_all_operations(const std::vector<OperationList> &bootstrapping_segments)
{
    auto segment_num = 0;
    for (const auto &segment : bootstrapping_segments)
    {
        for (auto &operation : segment)
        {
            operation->segment_nums.push_back(segment_num);
        }
        segment_num++;
    }
}

std::vector<OperationList> read_bootstrapping_segments(std::ifstream &input_file, OperationList &operations)
{
    std::vector<OperationList> bootstrapping_segments;
    std::string line;

    while (std::getline(input_file, line))
    {
        auto line_as_list = split_string_by_character(line, ',');

        if (line_as_list[0] == "")
        {
            continue;
        }

        auto segment_num = bootstrapping_segments.size();

        bootstrapping_segments.emplace_back();

        for (auto op_str : line_as_list)
        {
            auto op_num = std::stoi(op_str);
            auto op_ptr = get_operation_ptr_from_id(operations, op_num);
            op_ptr->segment_nums.push_back(segment_num);
            bootstrapping_segments.back().push_back(op_ptr);
        }
    }
    return bootstrapping_segments;
}