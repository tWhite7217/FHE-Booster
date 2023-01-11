#include <random>
#include <chrono>
#include <fstream>

#include "shared_utils.h"

class GraphGenerator
{
public:
    GraphGenerator();

    struct graph_generator_options
    {
        int num_operations;
        int max_num_starting_operations;
        double probability_of_two_parents;
    };

    void create_graph(graph_generator_options);
    void write_graph_to_txt_file(std::string);
    void write_graph_to_vcg_file(std::string);

private:
    OperationPtr add_random_operation_to_operations(int);
    void add_random_parents_to_operation(OperationPtr, double);
    std::string get_vcg_node_color(OperationPtr);

    std::minstd_rand rand_gen;
    OperationList operations;
};