#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include "shared_utils.h"
#include "program.h"
#include "LGRParser.h"

class InputParser
{
public:
    LatencyMap parse_latency_file(const std::string &);
    OpVector parse_dag_file(const std::string &);
    OpVector parse_dag_file_with_bootstrap_file(const std::string &, const std::string &);
    std::vector<BootstrapSegment> parse_segments_file(const std::string &);

private:
    OpVector operations;

    OperationType get_operation_type_from_string(const std::string &);
    void parse_operation_and_its_dependences(const std::vector<std::string> &);
    void parse_constant(std::vector<std::string>);
};
