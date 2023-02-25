#include "file_parser.h"

#include <algorithm>

LatencyMap FileParser::parse_latency_file(const std::string &latency_filename) const
{
    LatencyMap latencies;

    std::ifstream latency_file(latency_filename);

    auto line = utl::get_trimmed_line_from_file(latency_file);
    while (!line.empty())
    {
        auto line_as_list = utl::split_string_by_character(line, ',');
        auto type = OperationType(line_as_list[0]);
        latencies[type] = std::stoi(line_as_list[1]);
        line = utl::get_trimmed_line_from_file(latency_file);
    }

    return latencies;
}

std::shared_ptr<Program> FileParser::parse_dag_file(const std::string &dag_filename)
{
    return parse_dag_file_with_bootstrap_file(dag_filename, "");
}

std::shared_ptr<Program> FileParser::parse_dag_file_with_bootstrap_file(const std::string &dag_filename, const std::string &bootstrap_filename)
{
    program = std::unique_ptr<Program>(new Program());
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
        auto lgr_parser = LGRParser(bootstrap_filename, "-");
        lgr_parser.set_program(program);
        lgr_parser.lex();
    }

    return program;
}

void FileParser::parse_operation_and_its_dependences(const std::vector<std::string> &line)
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
            parent_ptr->child_ptrs.insert(new_operation);
        }
        else
        {
            new_operation->constant_parent_ids.push_back(parent_id);
        }
    }
}

void FileParser::parse_constant(const std::vector<std::string> &line)
{
    // operations.emplace_back(new Operation{type, int(operations.size()) + 1, parent_ptrs});
}

std::vector<BootstrapSegment> FileParser::parse_segments_file(const std::string &segments_filename) const
{
    std::ifstream segments_file(segments_filename, std::ios::binary);

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

    return get_segments_from_id_vector(ids, segment_sizes);
}

std::vector<BootstrapSegment> FileParser::get_segments_from_id_vector(const std::vector<int> &ids, const std::vector<size_t> &segment_sizes) const
{
    std::vector<BootstrapSegment> segments;
    int i = 0;
    for (const auto &seg_size : segment_sizes)
    {
        segments.emplace_back();
        auto &seg = segments.back();
        for (int j = 0; j < seg_size; j++)
        {
            seg.add(program->get_operation_ptr_from_id(ids[i]));
            i++;
        }
    }

    return segments;
}
