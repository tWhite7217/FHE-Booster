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