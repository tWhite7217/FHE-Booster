#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include "shared_utils.h"

class InputParser
{
public:
    // InputParser();
    void parse_input_to_generate_operations(std::string);

    std::map<std::string, int> get_operation_type_to_latency_map();
    OperationList get_operations();

private:
    OperationList operations;
    // ConstantList constants;
    std::map<std::string, int> operation_type_to_latency_map;
    std::vector<std::vector<OperationPtr>> bootstrapping_paths;

    void parse_lines(std::fstream &);
    void parse_operation_type(std::vector<std::string>);
    void parse_operation_and_its_dependences(std::vector<std::string>);
    void parse_constant(std::vector<std::string> line);
};
