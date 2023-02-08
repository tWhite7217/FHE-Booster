#include <algorithm>

#include "custom_ddg_format_parser.h"

std::map<std::string, int> InputParser::get_operation_type_to_latency_map() { return operation_type_to_latency_map; }
OperationList InputParser::get_operations() { return operations; }

void InputParser::parse_input_to_generate_operations(std::string filename)
{
    std::ifstream input_file;
    input_file.open(filename, std::ios::in);

    parse_lines(input_file);

    input_file.close();

    add_child_ptrs_to_operation_list_with_existing_parent_ptrs(operations);
}

void InputParser::parse_lines(std::ifstream &input_file)
{
    int phase = 0;
    std::string line;
    while (std::getline(input_file, line))
    {
        auto line_as_list = split_string_by_character(line, ',');

        if (line_as_list[0] == "")
        {
            return;
        }

        if (line_as_list[0] == "~")
        {
            phase++;
        }
        else if (phase == 0)
        {
            parse_operation_type(line_as_list);
        }
        else if (phase == 1)
        {
            parse_constant(line_as_list);
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
    std::vector<int> constant_parent_ids;
    for (int i = 2; i < line.size(); i++)
    {
        auto parent_is_ciphertext = (line[i][0] == 'c');
        auto parent_id = std::stoi(line[i].substr(1, line[i].length() - 1));
        if (parent_is_ciphertext)
        {
            auto parent_ptr = get_operation_ptr_from_id(operations, parent_id);
            parent_ptrs.push_back(parent_ptr);
        }
        else
        {
            constant_parent_ids.push_back(parent_id);
        }
    }
    operations.emplace_back(new Operation{type, int(operations.size()) + 1, parent_ptrs, constant_parent_ids});
}

void InputParser::parse_constant(std::vector<std::string> line)
{
    // operations.emplace_back(new Operation{type, int(operations.size()) + 1, parent_ptrs});
}