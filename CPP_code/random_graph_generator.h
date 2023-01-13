#include <random>
#include <chrono>
#include <fstream>

#include "shared_utils.h"

class RandomGraphGenerator
{
public:
    RandomGraphGenerator();

    struct graph_generator_options
    {
        int num_operations;
        int max_num_starting_operations;
        int min_num_constants;
        int max_num_constants;
        double probability_of_two_parents;
        double probability_parent_is_a_constant;
    };

    void create_graph(graph_generator_options);
    void write_graph_to_txt_file(std::string);

private:
    OperationPtr add_random_operation_to_operations(int);
    void add_random_parents_to_operation(OperationPtr, double, double);
    bool add_a_random_parent_type(std::vector<bool> &, const double &);
    bool operation_is_unique(OperationPtr);
    void fix_constants();

    std::minstd_rand rand_gen;
    OperationList operations;
    int num_constants;
    std::set<int> used_constants;
};