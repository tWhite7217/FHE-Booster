#include <random>
#include <chrono>
#include <fstream>
#include <numeric>

#include "shared_utils.h"
#include "operation.h"

class RandomGraphGenerator
{
public:
    RandomGraphGenerator();

    struct graph_generator_options
    {
        int min_operations;
        int min_levels;
        int max_levels;
        int min_width;
        int max_width;
        int max_constants;
        double probability_of_two_parents;
        double probability_parent_is_a_constant;
    };

    void create_graph(graph_generator_options);
    void write_graph_to_txt_file(std::string);

private:
    std::vector<int> get_random_level_widths(int);
    std::vector<OpVector> get_random_level_ops(std::vector<int> level_widths);
    void add_random_child_to_operation(OperationPtr, int);
    OperationPtr add_random_operation_to_operations(int);
    void add_random_parents_to_operation(OperationPtr, double, double, int);
    bool add_a_random_parent_type(std::vector<bool> &, const double &);
    bool operation_is_unique(OperationPtr);
    void fix_constants();

    graph_generator_options options;
    std::minstd_rand rand_gen;
    OpVector operations;
    std::vector<OpVector> level_ops;
    std::vector<int> level_widths;
    int num_constants;
    std::set<int> used_constants;
};