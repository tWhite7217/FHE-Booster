#include "program.h"

#include <algorithm>

Program::FileParser::FileParser(const std::reference_wrapper<Program> program_ref)
    : program_ref{program_ref} {}

void Program::FileParser::parse_latency_file(const std::string &latency_filename)
{
    std::ifstream latency_file(latency_filename);

    auto line = utl::get_trimmed_line_from_file(latency_file);
    auto &program = program_ref.get();
    while (!line.empty())
    {
        auto line_as_list = utl::split_string_by_character(line, ',');
        auto type = OperationType(line_as_list[0]);
        program.latencies[type] = std::stoi(line_as_list[1]);
        line = utl::get_trimmed_line_from_file(latency_file);
    }
}

void Program::FileParser::parse_dag_file(const std::string &dag_filename)
{
    parse_dag_file_with_bootstrap_file(dag_filename, "");
}

void Program::FileParser::parse_dag_file_with_bootstrap_file(const std::string &dag_filename, const std::string &bootstrap_filename)
{
    std::ifstream dag_file(dag_filename);
    auto line = utl::get_trimmed_line_from_file(dag_file);

    while (!line.empty() && line != "~")
    {
        auto line_as_list = utl::split_string_by_character(line, ',');

        parse_constant(line_as_list);

        line = utl::get_trimmed_line_from_file(dag_file);
    }

    line = utl::get_trimmed_line_from_file(dag_file);

    while (!line.empty())
    {
        auto line_as_list = utl::split_string_by_character(line, ',');
        parse_operation_and_its_dependences(line_as_list);
        line = utl::get_trimmed_line_from_file(dag_file);
    }

    if (!bootstrap_filename.empty())
    {
        auto lgr_parser = LGRParser(bootstrap_filename, "-", program_ref);
        lgr_parser.lex();
    }
}

void Program::FileParser::parse_operation_and_its_dependences(const std::vector<std::string> &line)
{
    auto type = OperationType(line[1]);
    auto &program = program_ref.get();
    auto new_operation = OperationPtr(new Operation(type, int(program.size()) + 1));
    program.add_operation(new_operation);

    for (size_t i = 2; i < line.size(); i++)
    {
        auto parent_is_ciphertext = (line[i][0] == 'c');
        auto parent_id = std::stoi(line[i].substr(1, line[i].length() - 1));
        if (parent_is_ciphertext)
        {
            auto parent_ptr = program.get_operation_ptr_from_id(parent_id);
            new_operation->parent_ptrs.push_back(parent_ptr);
            parent_ptr->child_ptrs.insert(new_operation);
        }
        else
        {
            new_operation->constant_parent_ids.push_back(parent_id);
        }
    }
}

void Program::FileParser::parse_constant(const std::vector<std::string> &line)
{
    // operations.emplace_back(new Operation{type, int(operations.size()) + 1, parent_ptrs});
}

void Program::FileParser::parse_segments_file(const std::string &segments_filename)
{
    std::ifstream segments_file(segments_filename);

    size_t num_segments;
    segments_file.read((char *)(&num_segments), sizeof(size_t));
    size_t total_num_ids = 0;

    std::vector<size_t> segment_sizes(num_segments);
    for (size_t i = 0; i < num_segments; i++)
    {
        size_t segment_length;
        segments_file.read((char *)(&segment_length), sizeof(size_t));
        segment_sizes[i] = segment_length;
        total_num_ids += segment_length;
    }

    std::vector<int> ids(total_num_ids);
    segments_file.read((char *)(&ids[0]), sizeof(int) * total_num_ids);

    generate_segments_from_id_vector(ids, segment_sizes);

    add_segment_index_info_to_operations();
}

void Program::FileParser::generate_segments_from_id_vector(const std::vector<int> &ids, const std::vector<size_t> &segment_sizes)
{
    int i = 0;
    auto &program = program_ref.get();
    for (const auto &seg_size : segment_sizes)
    {
        program.bootstrap_segments.emplace_back();
        auto &seg = program.bootstrap_segments.back();
        for (size_t j = 0; j < seg_size; j++)
        {
            seg.add(program.get_operation_ptr_from_id(ids[i]));
            i++;
        }
    }
}

void Program::FileParser::add_segment_index_info_to_operations()
{
    auto segment_index = 0;
    for (const auto &segment : program_ref.get().bootstrap_segments)
    {
        for (auto &operation : segment)
        {
            operation->segment_indexes.insert(segment_index);
        }
        segment_index++;
    }
}