#include "custom_ddg_format_parser.h"

const int bootstrapping_latency = 12;
const int bootstrapping_path_length = 3;
const int num_bootstrapping_cores = 1;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cout << "Usage: " << argv[0] << " <input_file> <output_file> <allow_bootstrapping_to_only_some_children>" << std::endl;
        return 1;
    }

    auto input_filename = std::string{argv[1]};
    auto output_filename = std::string{argv[2]};
    bool allow_bootstrapping_to_only_some_children = (std::string(argv[3]) == "True");

    auto input_parser = InputParser(bootstrapping_path_length, false, allow_bootstrapping_to_only_some_children);
    input_parser.parse_input(input_filename);

    auto operation_type_to_latency_map = input_parser.get_operation_type_to_latency_map();
    auto operations = input_parser.get_operations();
    auto bootstrapping_paths = input_parser.get_bootstrapping_paths();

    std::fstream output_file;
    output_file.open(output_filename, std::ios::out);

    int i = 0;
    for (auto [operation_type, latency] : operation_type_to_latency_map)
    {
        output_file << "T" << i << " " << latency << std::endl;
        i++;
    }

    output_file << "~" << std::endl;

    for (auto i = 1; i <= operations.size(); i++)
    {
        output_file << "OP" << i << std::endl;
    }

    output_file << "~" << std::endl;

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

    output_file << "~" << std::endl;

    int operation_id = 1;
    for (auto operation : operations)
    {
        for (auto parent_id : operation.parent_ids)
        {
            output_file << "OP" << parent_id << " OP" << operation_id << std::endl;
        }
        operation_id++;
    }

    output_file << "~" << std::endl;

    output_file << bootstrapping_latency << std::endl;

    output_file << "~" << std::endl;

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

    output_file << "~" << std::endl;

    if (num_bootstrapping_cores > 0)
    {
        output_file << "C1..C" << num_bootstrapping_cores << std::endl;
    }

    output_file.close();
}