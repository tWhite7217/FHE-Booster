#include "custom_ddg_format_parser.h"

#include <functional>

const int bootstrapping_latency = 12;
const int bootstrapping_path_length = 3;

bool allow_bootstrapping_to_only_some_children;
std::string input_file_path;
std::string output_file_path;

std::map<std::string, int> operation_type_to_latency_map;
std::vector<Operation> operations;
std::vector<std::vector<int>> bootstrapping_paths;

std::fstream output_file;

void read_command_line_args(int argc, char **argv)
{
    input_file_path = std::string{argv[1]};
    output_file_path = std::string{argv[2]};
    allow_bootstrapping_to_only_some_children = (std::string(argv[3]) == "True");
}

void get_info_from_input_parser()
{
    auto input_parser = InputParser(bootstrapping_path_length, false, allow_bootstrapping_to_only_some_children);
    input_parser.parse_input(input_file_path);

    operation_type_to_latency_map = input_parser.get_operation_type_to_latency_map();
    operations = input_parser.get_operations();
    bootstrapping_paths = input_parser.get_bootstrapping_paths();
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
        auto operation_type_num = std::distance(operation_type_to_latency_map.begin(), operation_type_to_latency_map.find(operation.type));
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
    int operation_id = 1;
    for (auto operation : operations)
    {
        for (auto parent_id : operation.parent_ids)
        {
            output_file << "OP" << parent_id << " OP" << operation_id << std::endl;
        }
        operation_id++;
    }
}

void write_bootstrapping_latency_to_output_file()
{
    output_file << bootstrapping_latency << std::endl;
}

void write_bootstrapping_constraints_to_output_file()
{
    for (auto path : bootstrapping_paths)
    {
        std::string constraint_string;
        if (allow_bootstrapping_to_only_some_children)
        {
            for (auto i = 0; i < path.size() - 1; i++)
            {
                constraint_string += "BOOTSTRAPPED(" + std::to_string(path[i]) + ", " + std::to_string(path[i + 1]) + ") + ";
            }
        }
        else
        {
            for (auto i = 0; i < path.size(); i++)
            {
                constraint_string += "BOOTSTRAPPED(" + std::to_string(path[i]) + ") + ";
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
    if (argc != 4)
    {
        std::cout << "Usage: " << argv[0] << " <input_file> <output_file> <allow_bootstrapping_to_only_some_children>" << std::endl;
        return 1;
    }

    read_command_line_args(argc, argv);
    get_info_from_input_parser();

    output_file.open(output_file_path, std::ios::out);

    std::vector<std::function<void()>> write_functions = {
        write_operation_type_latencies_to_output_file,
        write_operation_list_to_output_file,
        write_operation_types_to_output_file,
        write_operation_dependencies_to_output_file,
        write_bootstrapping_latency_to_output_file,
        write_bootstrapping_constraints_to_output_file};

    for (auto write_function : write_functions)
    {
        write_function();
        write_data_separator_to_file();
    }

    output_file.close();
}