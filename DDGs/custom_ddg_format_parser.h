#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include "../shared_utils.h"

class InputParser
{
public:
    InputParser(int, bool, bool, int);
    void parse_input(std::string);
    std::map<std::string, int> get_operation_type_to_latency_map();
    std::vector<Operation> get_operations();
    std::vector<std::vector<int>> get_bootstrapping_paths();

private:
    int bootstrapping_path_threshold;
    int addition_divider;
    std::map<std::string, int> operation_type_to_latency_map;
    bool is_for_validation;
    bool allow_bootstrapping_to_some_children_only;
    std::vector<Operation> operations;
    std::vector<std::vector<int>> bootstrapping_paths;

    Operation get_operation_from_id(int);
    void parse_lines(std::fstream &);
    void parse_operation_type(std::vector<std::string>);
    void parse_operation_and_its_dependences(std::vector<std::string>);
    std::vector<std::string> get_string_list_from_line(std::string);
    void create_bootstrapping_paths();
    std::vector<std::vector<int>> create_bootstrapping_paths_helper(int, int, int);
    void create_bootstrapping_paths_for_validation();
    std::vector<std::vector<int>> depth_first_search(std::vector<int>, std::vector<std::vector<int>>);
    void remove_last_operation_from_bootstrapping_paths();
    void remove_duplicate_bootstrapping_paths();
    void remove_redundant_bootstrapping_paths();
    bool path_is_redundant(size_t);
    bool larger_path_contains_smaller_path(std::vector<int>, std::vector<int>);
    float get_path_cost(std::vector<int>);
};
