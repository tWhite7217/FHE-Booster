#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include "program.h"
#include "LGRParser.h"

class Program::FileParser
{
public:
    FileParser(const std::reference_wrapper<Program>);
    void parse_latency_file(const std::string &);
    void parse_dag_file(const std::string &);
    void parse_dag_file_with_bootstrap_file(const std::string &, const std::string &);
    void parse_segments_file(const std::string &);

private:
    std::reference_wrapper<Program> program_ref;

    void parse_operation_and_its_dependences(const std::vector<std::string> &);
    void parse_constant(const std::vector<std::string> &);
    void generate_segments_from_id_vector(const std::vector<int> &, const std::vector<size_t> &);
    void add_segment_index_info_to_operations();
};
