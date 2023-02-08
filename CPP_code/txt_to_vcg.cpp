#include "../CPP_code/custom_ddg_format_parser.h"

#include <functional>

std::string input_filename;
std::string output_filename;

OperationList operations;
std::set<int> constant_ids;

void read_command_line_args(int argc, char **argv)
{
    input_filename = std::string{argv[1]};
    output_filename = std::string{argv[2]};
}

void get_info_from_input_parser()
{
    InputParser input_parser;
    input_parser.parse_input_to_generate_operations(input_filename);
    operations = input_parser.get_operations();

    for (auto operation : operations)
    {
        for (auto constant_id : operation->constant_parent_ids)
        {
            constant_ids.insert(constant_id);
        }
    }
}

std::string get_vcg_node_color(OperationPtr operation)
{
    // if (operation->parent_ptrs.size() == 0)
    // {
    //     return "lightgreen";
    // } else
    if (operation->child_ptrs.size() > 0)
    {
        return "white";
    }
    else
    {
        return "lightred";
    }
}

void write_graph_to_vcg_file(std::string output_filename)
{
    std::ofstream output_file(output_filename);

    output_file << "graph: { title: \"Graph\"" << std::endl;

    for (auto &constant_id : constant_ids)
    {
        output_file << "node: {title: \"K" << constant_id << "\" label: \"SET CONST" << constant_id << "\" color: lightgreen }" << std::endl;
    }

    for (auto &operation : operations)
    {
        std::string color = get_vcg_node_color(operation);
        output_file << "node: {title: \"" << operation->id << "\" label: \"OP" << operation->id << " (" << operation->type << ")\" color: " << color << " }" << std::endl;
    }

    for (auto &operation : operations)
    {
        for (auto parent : operation->parent_ptrs)
        {
            output_file << "edge: {sourcename: \"" << parent->id << "\" targetname: \"" << operation->id << "\" }" << std::endl;
        }
        for (auto constant_id : operation->constant_parent_ids)
        {
            output_file << "edge: {sourcename: \"K" << constant_id << "\" targetname: \"" << operation->id << "\" }" << std::endl;
        }
    }

    output_file << "}" << std::endl;

    output_file.close();
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
        return 1;
    }

    read_command_line_args(argc, argv);
    get_info_from_input_parser();

    write_graph_to_vcg_file(output_filename);
}