#include "custom_ddg_format_parser.h"

#include <functional>

bool using_selective_model;
std::string dag_file_path;
std::string bootstrap_file_path;
std::string output_file_path;
int gained_levels;

std::map<std::string, int> operation_type_to_latency_map;
OperationList operations;
std::vector<std::vector<OperationPtr>> bootstrap_segments;

std::fstream output_file;

void read_command_line_args(int argc, char **argv)
{
    dag_file_path = argv[1];
    bootstrap_file_path = argv[2];
    output_file_path = argv[3];
    using_selective_model = (std::string(argv[4]) == "True");
    gained_levels = std::stoi(argv[5]);
}

void get_info_from_input_parser()
{
    InputParser input_parser;
    input_parser.parse_input_to_generate_operations(dag_file_path);
    operation_type_to_latency_map = input_parser.get_operation_type_to_latency_map();
    operations = input_parser.get_operations();
}

void write_data_separator_to_file()
{
    output_file << "~" << std::endl;
}

void write_operation_type_latencies_to_output_file()
{
    int i = 0;
    for (auto [operation_type, latency] : operation_type_to_latency_map)
    {
        output_file << "T" << i << " " << latency << std::endl;
        i++;
    }
}

void write_operation_list_to_output_file()
{
    for (auto i = 1; i <= operations.size(); i++)
    {
        output_file << "OP" << i << std::endl;
    }
}

void write_operation_types_to_output_file()
{
    for (auto operation : operations)
    {
        auto operation_type_num = std::distance(operation_type_to_latency_map.begin(), operation_type_to_latency_map.find(operation->type));
        for (auto i = 0; i < operation_type_to_latency_map.size(); i++)
        {
            if (i == operation_type_num)
            {
                output_file << "1 ";
            }
            else
            {
                output_file << "0 ";
            }
        }
        output_file << std::endl;
    }
}

void write_operation_dependencies_to_output_file()
{
    for (auto operation : operations)
    {
        for (auto parent : operation->parent_ptrs)
        {
            output_file << "OP" << parent->id << " OP" << operation->id << std::endl;
        }
    }
}

void write_bootstrap_latency_to_output_file()
{
    output_file << bootstrap_latency << std::endl;
}

void write_bootstrapping_constraints_to_output_file()
{
    for (auto segment : bootstrap_segments)
    {
        std::string constraint_string;
        if (using_selective_model)
        {
            for (auto i = 0; i < segment.size() - 1; i++)
            {
                if ((i > 0) && (i % 10 == 0))
                {
                    constraint_string += "\n";
                }
                constraint_string += "BOOTSTRAPPED(" + std::to_string(segment[i]->id) + ", " + std::to_string(segment[i + 1]->id) + ") + ";
            }
        }
        else
        {
            for (auto i = 0; i < segment.size(); i++)
            {
                if ((i > 0) && (i % 20 == 0))
                {
                    constraint_string += "\n";
                }
                constraint_string += "BOOTSTRAPPED(" + std::to_string(segment[i]->id) + ") + ";
            }
        }
        constraint_string.pop_back();
        constraint_string.pop_back();
        constraint_string += ">= 1;";
        output_file << constraint_string << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        std::cout << "Usage: " << argv[0] << " <dag_file> <bootstrap_file> <output_file> <using_selective_model> <gained_levels>" << std::endl;
        return 1;
    }

    read_command_line_args(argc, argv);
    get_info_from_input_parser();

    std::ifstream bootstrap_file(bootstrap_file_path);
    bootstrap_segments = read_bootstrap_segments(bootstrap_file, operations);

    output_file.open(output_file_path, std::ios::out);

    std::vector<std::function<void()>> write_functions = {
        // write_operation_type_latencies_to_output_file,
        write_operation_list_to_output_file,
        write_operation_types_to_output_file,
        write_operation_dependencies_to_output_file,
        // write_bootstrap_latency_to_output_file,
        write_bootstrapping_constraints_to_output_file};

    for (auto write_function : write_functions)
    {
        write_function();
        write_data_separator_to_file();
    }

    output_file.close();
}