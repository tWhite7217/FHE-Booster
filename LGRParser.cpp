#include <algorithm>

#include "LGRParser.h"

void LGRParser::remove_chars_from_string(std::string &str, std::vector<char> chars_to_remove)
{
    for (unsigned int i = 0; i < chars_to_remove.size(); ++i)
    {
        str.erase(remove(str.begin(), str.end(), chars_to_remove[i]), str.end());
    }
}

int LGRParser::extract_number_from_string(std::string str, size_t start_index, size_t end_index)
{
    auto num_digits = end_index - start_index;
    std::string num_as_string = str.substr(start_index, num_digits);
    int num = std::stoi(num_as_string);
    return num;
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