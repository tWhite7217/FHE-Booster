#include <random>
#include <chrono>
#include <fstream>
#include <numeric>

#include "shared_utils.h"
#include "program.h"

class RandomGraphGenerator
{
public:
    RandomGraphGenerator(int, char **);

    void create_graph();
    void write_graph_to_txt_file(); // move to file_writer?

private:
    struct graph_generator_options
    {
        std::string output_file_base_path;
        int min_operations;
        int min_levels;
        int max_levels;
        int min_width;
        int max_width;
        int max_constants;
        double probability_of_two_parents = 0.7;
        double constant_probability = 0.02;
        bool use_input_seed;
        bool save_seed;
    } options;

    static const std::string help_info;

    void parse_args(int, char **);
    void print_options() const;

    unsigned int read_input_seed() const;
    void write_seed(unsigned int) const;

    std::vector<int> get_random_level_widths(const int);
    std::vector<OpVector> get_random_level_ops(const std::vector<int> &level_widths);
    void add_random_child_to_operation(const OperationPtr &, const int);
    OperationPtr add_random_operation_to_operations(const int);
    void add_random_parents_to_operation(const OperationPtr &, const double, const double, const int);
    bool add_a_random_parent_type(std::vector<bool> &, const double);
    bool operation_is_unique(const OperationPtr &) const;
    void fix_constants();

    std::minstd_rand rand_gen;

    Program program;
    std::vector<OpVector> level_ops;
    std::vector<int> level_widths;
    int num_constants;
    std::set<int> used_constants;
};

const std::string RandomGraphGenerator::help_info = R"(
Usage: ./bootstrap_set_selector <ouptut_file_base_path=file>
                                <min_num_operations=int>
                                <min_levels=int>
                                <max_levels=int>
                                <min_width=int>
                                <max_width=int>
                                [-i]
                                [-s]
                                [-m <max_constants=int>]
                                [-p <probability_of_two_parents=float>]
                                [-c <probability_parent_is_a_constant=float>]

Arguments:
  <ouptut_file_base_path=file>
    The path of the output file, without the file extension.
    The graph will be saved to <ouptut_file_base_path>.txt
  <min_num_operations=int>
    The minimum number of operations the random graph must have.
    The actual number of operations may exceed this slightly.
  <min_levels=int>
    The minimum number of levels that the random graph must have.
  <max_levels=int>
    The maximum number of levels that the random graph may have.
  <min_width=int>
    The minimum width of any given levels in the random graph.
  <max_width=int>
    The maximum width of any given levels in the random graph.
  -i, --input-seed
    The graph will be generated using a previously created seed.
    That seed must be in the file at <ouptut_file_base_path>.seed
  -s, --save-seed
    The used seed will be saved to the file
    <ouptut_file_base_path>.seed
  -m <int>, --max-constants=<int>
    The maximum number of ciphertext inputs the random graph
    can have. Defaults to <max_width>.
  -p <float>, --probability-of-two-parents=<float>
    The probabilty that an operation will be given two inputs.
    Defaults to 0.7.
  -c <float>, --constant-probability=<float>
    The probability that an input to any given operation will
    be one of the initial ciphertext inputs. Defaults to 0.02.
        
Note:
  The supplied arguments must satisfy the following constraints.
  
  min_operations >= 5
  min_levels * max_width >= min_operations
  max_levels * min_width <= min_operations
  max_constants >= max_width / 2
  
  The program will check that these constraints are met and will
  stop execution if not.)";