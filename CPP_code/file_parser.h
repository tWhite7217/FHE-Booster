#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include "LGRParser.h"
#include "shared_utils.h"
#include "program.h"

class Program;

class FileParser
{
public:
    LatencyMap parse_latency_file(const std::string &) const;
    std::shared_ptr<Program> parse_dag_file(const std::string &);
    std::shared_ptr<Program> parse_dag_file_with_bootstrap_file(const std::string &, const std::string &);
    std::vector<BootstrapSegment> parse_segments_file(const std::string &) const;

private:
    std::shared_ptr<Program> program;

    void parse_operation_and_its_dependences(const std::vector<std::string> &);
    void parse_constant(const std::vector<std::string> &);
    std::vector<BootstrapSegment> get_segments_from_id_vector(const std::vector<int> &, const std::vector<size_t> &) const;
};
