#include <algorithm>

#include "custom_ddg_format_parser.h"

std::map<std::string, int> InputParser::get_operation_type_to_latency_map() { return operation_type_to_latency_map; }
OperationList InputParser::get_operations() { return operations; }

void InputParser::parse_input_to_generate_operations(std::string filename)
{
    std::fstream input_file;
    input_file.open(filename, std::ios::in);

    parse_lines(input_file);

    input_file.close();
}

void InputParser::parse_lines(std::fstream &input_file)
{
    int phase = 0;
    std::string line;
    while (std::getline(input_file, line))
    {
        auto line_as_list = get_string_list_from_line(line);

        if (line_as_list[0] == "")
        {
            return;
        }

        if (line_as_list[0] == "~")
        {
            phase = 1;
        }
        else if (phase == 0)
        {
            parse_operation_type(line_as_list);
        }
        else
        {
            parse_operation_and_its_dependences(line_as_list);
        }
    }
}

void InputParser::parse_operation_type(std::vector<std::string> line)
{
    operation_type_to_latency_map[line[0]] = std::stoi(line[1]);
}

void InputParser::parse_operation_and_its_dependences(std::vector<std::string> line)
{
    std::string type = line[1];
    std::vector<OperationPtr> parent_ptrs;
    for (int i = 2; i < line.size(); i++)
    {
        auto parent_id = std::stoi(line[i]);
        auto parent_ptr = get_operation_ptr_from_id(operations, parent_id);
        parent_ptrs.push_back(parent_ptr);
    }
    operations.emplace_back(new Operation{type, int(operations.size()) + 1, parent_ptrs});
}

std::vector<std::string> InputParser::get_string_list_from_line(std::string line)
{
    std::vector<std::string> line_as_list;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, ','))
    {
        line_as_list.push_back(item);
    }
    return line_as_list;
}