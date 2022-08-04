#include <algorithm>

#include "LGRParser.h"

void LGRParser::set_operations(OperationList &operations) { this->operations = operations; }

OperationPtr LGRParser::get_first_operation_ptr(std::string yystring)
{
    int operation_id = get_first_operation_id(yystring);
    return get_operation_ptr_from_id(operations, operation_id);
}

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

OperationPtr LGRParser::get_second_operation_ptr(std::string yystring)
{
    int operation_id = get_second_operation_id(yystring);
    return get_operation_ptr_from_id(operations, operation_id);
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

bool LGRParser::operation_is_bootstrapped(OperationPtr operation)
{
    return operation_is_bootstrapped(operation, nullptr);
}

bool LGRParser::operation_is_bootstrapped(OperationPtr operation, OperationPtr child)
{
    if (used_selective_model)
    {
        std::function<bool(int)> checker = get_checker_for_selective_model(operation, child);

        for (auto i = 0; i < bootstrapped_operations.size(); i += 2)
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
        return vector_contains_element(bootstrapped_operations, operation);
    }
}

std::function<bool(int)> LGRParser::get_checker_for_selective_model(OperationPtr operation, OperationPtr child)
{
    if (child == nullptr)
    {
        return [&, this](int i)
        {
            return bootstrapped_operations[i] == operation;
        };
    }
    else
    {
        return [&, this](int i)
        {
            return bootstrapped_operations[i] == operation &&
                   bootstrapped_operations[i + 1] == child;
        };
    }
}