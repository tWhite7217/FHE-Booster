#include "custom_ddg_format_parser.h"
#include "bootstrapping_path_generator.h"

#include <functional>

bool using_selective_model;
std::string input_file_path;
std::string output_file_path;

std::map<std::string, int> operation_type_to_latency_map;
OperationList operations;
std::vector<std::vector<OperationPtr>> bootstrapping_paths;

std::fstream output_file;

void read_command_line_args(int argc, char **argv)
{
    input_file_path = std::string{argv[1]};
    output_file_path = std::string{argv[2]};
    using_selective_model = (std::string(argv[3]) == "True");
}

void get_info_from_input_parser()
{
    InputParser input_parser;
    input_parser.parse_input_to_generate_operations(input_file_path);
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

void write_bootstrapping_latency_to_output_file()
{
    output_file << bootstrapping_latency << std::endl;
}

void write_bootstrapping_constraints_to_output_file()
{
    for (auto path : bootstrapping_paths)
    {
        std::string constraint_string;
        if (using_selective_model)
        {
            for (auto i = 0; i < path.size() - 1; i++)
            {
                constraint_string += "BOOTSTRAPPED(" + std::to_string(path[i]->id) + ", " + std::to_string(path[i + 1]->id) + ") + ";
            }
        }
        else
        {
            for (auto i = 0; i < path.size(); i++)
            {
                constraint_string += "BOOTSTRAPPED(" + std::to_string(path[i]->id) + ") + ";
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
        std::cout << "Usage: " << argv[0] << " <input_file> <output_file> <using_selective_model>" << std::endl;
        return 1;
    }

    read_command_line_args(argc, argv);
    get_info_from_input_parser();

    BootstrappingPathGenerator path_generator(operations, using_selective_model);
    bootstrapping_paths = path_generator.generate_bootstrapping_paths();
    // bootstrapping_paths = path_generator.generate_bootstrapping_paths_for_validation();

    output_file.open(output_file_path, std::ios::out);

    std::vector<std::function<void()>> write_functions = {
        // write_operation_type_latencies_to_output_file,
        write_operation_list_to_output_file,
        write_operation_types_to_output_file,
        write_operation_dependencies_to_output_file,
        // write_bootstrapping_latency_to_output_file,
        write_bootstrapping_constraints_to_output_file};

    for (auto write_function : write_functions)
    {
        write_function();
        write_data_separator_to_file();
    }

    output_file.close();
}