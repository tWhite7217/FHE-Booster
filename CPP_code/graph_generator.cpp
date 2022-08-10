#include "graph_generator.h"

GraphGenerator::GraphGenerator()
{
    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    rand_gen.seed(seed);
}

void GraphGenerator::create_graph(graph_generator_options options)
{
    int num_starting_operations = rand_gen() % options.max_num_starting_operations + 1;

    for (int i = 0; i < num_starting_operations; i++)
    {
        add_random_operation_to_operations(i + 1);
    }

    for (int i = num_starting_operations; i < options.num_operations; i++)
    {
        auto operation = add_random_operation_to_operations(i + 1);

        add_random_parents_to_operation(operation, options.probability_of_two_parents);
    }

    add_child_ptrs_to_operation_list_with_existing_parent_ptrs(operations);
}

OperationPtr GraphGenerator::add_random_operation_to_operations(int operation_id)
{
    int operation_type_num = rand_gen() % 2;
    std::string operation_type = operation_type_num == 0 ? "ADD" : "MUL";
    operations.emplace_back(new Operation{operation_type, operation_id});
    return operations.back();
}

void GraphGenerator::add_random_parents_to_operation(OperationPtr operation, double two_parent_probability)
{
    double rand_decimal = ((double)rand_gen()) / RAND_MAX;
    int num_parents = rand_decimal > two_parent_probability ? 1 : 2;

    int i = 0;
    int prev_parent_index = -1;
    while (i < num_parents)
    {
        int parent_index = rand_gen() % (operations.size() - 1);
        if (parent_index != prev_parent_index)
        {
            auto parent_ptr = operations[parent_index];
            operation->parent_ptrs.push_back(parent_ptr);
            prev_parent_index = parent_index;
            i++;
        }
    }
}

void GraphGenerator::write_graph_to_txt_file(std::string output_file_path)
{
    std::ofstream output_file(output_file_path);

    // for (auto operation : operations)
    // {
    //     output_file << operation->type << "," << latency << std::endl;
    // }
    output_file << "ADD,1" << std::endl;
    output_file << "MUL,5" << std::endl;

    output_file << "~" << std::endl;

    for (auto &operation : operations)
    {
        output_file << operation->id << ",";
        output_file << operation->type;
        if (operation->parent_ptrs.size() > 0)
        {
            std::string dependency_string = ",";
            for (auto parent : operation->parent_ptrs)
            {
                dependency_string += std::to_string(parent->id) + ",";
            }
            dependency_string = dependency_string.substr(0, dependency_string.size() - 1);
            output_file << dependency_string;
        }
        output_file << std::endl;
    }
}

void GraphGenerator::write_graph_to_vcg_file(std::string output_file_path)
{
    std::ofstream output_file(output_file_path);

    output_file << "graph: { title: \"Graph\"" << std::endl;

    for (auto &operation : operations)
    {
        std::string color = get_vcg_node_color(operation);
        output_file << "node: {title: \"" << operation->id << "\" label: \"OP" << operation->id << " (" << operation->type << ")\" color: " << color << " }" << std::endl;
    }

    for (auto &operation : operations)
    {
        for (auto child : operation->child_ptrs)
        {
            output_file << "edge: {sourcename: \"" << operation->id << "\" targetname: \"" << child->id << "\" }" << std::endl;
        }
    }

    output_file << "}" << std::endl;
}

std::string GraphGenerator::get_vcg_node_color(OperationPtr operation)
{
    if (operation->parent_ptrs.size() == 0)
    {
        return "lightgreen";
    }
    else if (operation->child_ptrs.size() > 0)
    {
        return "white";
    }
    else
    {
        return "lightred";
    }
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        std::cout << "Usage: ./graph_generator <num_operations> <max_num_starting_operations> <probability_of_two_parents> <output_file_base_path>" << std::endl;
        return 1;
    }

    int num_operations = std::stoi(argv[1]);
    int max_num_starting_operations = std::stoi(argv[2]);
    double probability_of_two_parents = std::stod(argv[3]);
    std::string output_file_base_path = argv[4];

    auto options = GraphGenerator::graph_generator_options{
        num_operations,
        max_num_starting_operations,
        probability_of_two_parents};

    GraphGenerator graph_generator;
    graph_generator.create_graph(options);
    std::cout << "Graph created" << std::endl;
    graph_generator.write_graph_to_txt_file(output_file_base_path + ".txt");
    std::cout << "txt created" << std::endl;
    graph_generator.write_graph_to_vcg_file(output_file_base_path + ".vcg");
}