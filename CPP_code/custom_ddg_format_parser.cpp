#include "custom_ddg_format_parser.h"

#include <algorithm>

LatencyMap InputParser::parse_latency_file(const std::string &latency_filename)
{
    LatencyMap latencies;

    std::ifstream latency_file(latency_filename);

    std::string line;
    while (std::getline(latency_file, line) && line != "")
    {
        auto line_as_list = split_string_by_character(line, ',');

        // if (line_as_list[0] == "")
        // {
        //     break;
        // }

        auto type = OperationType(line_as_list[0]);
        latencies[type] = std::stoi(line_as_list[1]);
    }

    return latencies;
}

std::shared_ptr<Program> InputParser::parse_dag_file(const std::string &dag_filename)
{
    return parse_dag_file_with_bootstrap_file(dag_filename, "");
}

std::shared_ptr<Program> InputParser::parse_dag_file_with_bootstrap_file(const std::string &dag_filename, const std::string &bootstrap_filename)
{
    program = std::unique_ptr<Program>(new Program());
    std::ifstream dag_file(dag_filename);
    std::string line;

    while (std::getline(dag_file, line) && line != "~" && line != "")
    {
        auto line_as_list = split_string_by_character(line, ',');

        // if (line_as_list[0] == "")
        // {
        //     return;
        // }

        // if (line_as_list[0] == "~")
        // {
        //     phase++;
        // }
        // else if (phase == 0)
        // {
        parse_constant(line_as_list);
        // }
        // else
        // {
        // parse_operation_and_its_dependences(line_as_list);
        // }
    }

    while (std::getline(dag_file, line) && line != "")
    {
        auto line_as_list = split_string_by_character(line, ',');
        parse_operation_and_its_dependences(line_as_list);
    }

    if (!bootstrap_filename.empty())
    {
        auto lgr_parser = LGRParser(bootstrap_filename, "-");
        lgr_parser.set_program(program);
        lgr_parser.lex();
    }

    return program;
}

void InputParser::parse_operation_and_its_dependences(const std::vector<std::string> &line)
{
    auto type = OperationType(line[1]);
    auto new_operation = OperationPtr(new Operation(type, int(program->size()) + 1));
    program->add_operation(new_operation);

    for (int i = 2; i < line.size(); i++)
    {
        auto parent_is_ciphertext = (line[i][0] == 'c');
        auto parent_id = std::stoi(line[i].substr(1, line[i].length() - 1));
        if (parent_is_ciphertext)
        {
            auto parent_ptr = program->get_operation_ptr_from_id(parent_id);
            new_operation->parent_ptrs.push_back(parent_ptr);
            parent_ptr->child_ptrs.push_back(new_operation);
        }
        else
        {
            new_operation->constant_parent_ids.push_back(parent_id);
        }
    }
}

void InputParser::parse_constant(std::vector<std::string> line)
{
    // operations.emplace_back(new Operation{type, int(operations.size()) + 1, parent_ptrs});
}

std::vector<BootstrapSegment> InputParser::parse_segments_file(const std::string &segments_filename)
{
    std::ifstream segments_file(segments_filename);
    std::vector<BootstrapSegment> bootstrap_segments;
    std::string line;

    while (std::getline(segments_file, line) && line != "")
    {
        auto line_as_list = split_string_by_character(line, ',');

        // if (line_as_list[0] == "")
        // {
        //     continue;
        // }

        auto segment_index = bootstrap_segments.size();

        bootstrap_segments.emplace_back();

        for (auto op_str : line_as_list)
        {
            auto op_id = std::stoi(op_str);
            auto op_ptr = program->get_operation_ptr_from_id(op_id);
            op_ptr->segment_indexes.push_back(segment_index);
            bootstrap_segments.back().add(op_ptr);
        }
    }
    return bootstrap_segments;
}
