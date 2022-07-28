#include <algorithm>

#include "LGRParser.h"

int LGRParser::get_first_operation_id(std::string yystring)
{
    auto operation_start_index = yystring.find("OP") + 2;
    auto operation_end_index = yystring.find(",");
    if (operation_end_index == std::string::npos)
    {
        operation_end_index = yystring.find(")");
    }
    return extract_number_from_string(yystring, operation_start_index, operation_end_index);
}

int LGRParser::get_second_operation_id(std::string yystring)
{
    auto operation_start_index = yystring.rfind("OP") + 2;
    auto operation_end_index = yystring.find(")");
    return extract_number_from_string(yystring, operation_start_index, operation_end_index);
}

int LGRParser::get_core_num(std::string yystring)
{
    auto core_start_index = yystring.find(", C") + 3;
    auto core_end_index = yystring.find(")");
    return extract_number_from_string(yystring, core_start_index, core_end_index);
}

int LGRParser::get_time(std::string yystring)
{
    remove_chars_from_string(yystring, {' ', '\t'});
    auto time_start_index = yystring.find(")") + 1;
    auto time_end_index = yystring.size();
    return extract_number_from_string(yystring, time_start_index, time_end_index);
}

bool LGRParser::operations_bootstrap_on_same_core(int op_id1, int op_id2)
{
    return cores_used[op_id1] == cores_used[op_id2];
}

bool LGRParser::operation_is_bootstrapped(int operation_id)
{
    return operation_is_bootstrapped(operation_id, -1);
}

bool LGRParser::operation_is_bootstrapped(int operation_id, int child_id)
{
    if (used_selective_model)
    {
        std::function<bool(int)> checker = get_checker_for_selective_model(operation_id, child_id);

        for (auto i = 0; i < bootstrapped_operation_ids.size(); i += 2)
        {
            if (checker(i))
            {
                return true;
            }
        }
        return false;
    }
    else
    {
        return vector_contains_element(bootstrapped_operation_ids, operation_id);
    }
}

std::function<bool(int)> LGRParser::get_checker_for_selective_model(int operation_id, int child_id)
{
    if (child_id == -1)
    {
        return [&, this](int i)
        {
            return bootstrapped_operation_ids[i] == operation_id;
        };
    }
    else
    {
        return [&, this](int i)
        {
            return bootstrapped_operation_ids[i] == operation_id &&
                   bootstrapped_operation_ids[i + 1] == child_id;
        };
    }
}

void LGRParser::add_info_to_operation_list(OperationList &operations)
{
    for (auto [operation_id, start_time] : start_times)
    {
        operations.get(operation_id).start_time = start_time;
    }

    for (auto [operation_id, bootstrap_start_time] : bootstrap_start_times)
    {
        operations.get(operation_id).bootstrap_start_time = bootstrap_start_time;
    }

    for (auto [operation_id, core_num] : cores_used)
    {
        operations.get(operation_id).core_num = core_num;
    }
}