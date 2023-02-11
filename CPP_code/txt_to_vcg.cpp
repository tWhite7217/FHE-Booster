#include "../CPP_code/custom_ddg_format_parser.h"

#include <functional>

std::string input_filename;
std::string output_filename;

OpVector operations;
std::set<int> constant_ids;

void read_command_line_args(int argc, char **argv)
{
    input_filename = std::string{argv[1]};
    output_filename = std::string{argv[2]};
}

void get_info_from_input_parser()
{
    InputParser input_parser;
    operations = input_parser.parse_dag_file(input_filename);

    for (auto operation : operations)
    {
        for (const auto &constant_id : operation->get_constant_parent_ids())
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
    if (operation->get_child_ptrs().size() > 0)
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
        output_file << "node: {title: \"" << operation->get_id() << "\" label: \"OP" << operation->get_id() << " (" << operation->get_type() << ")\" color: " << color << " }" << std::endl;
    }

    for (auto &operation : operations)
    {
        for (const auto &parent : operation->get_parent_ptrs())
        {
            output_file << "edge: {sourcename: \"" << parent->get_id() << "\" targetname: \"" << operation->get_id() << "\" }" << std::endl;
        }
        for (const auto &constant_id : operation->get_constant_parent_ids())
        {
            output_file << "edge: {sourcename: \"K" << constant_id << "\" targetname: \"" << operation->get_id() << "\" }" << std::endl;
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