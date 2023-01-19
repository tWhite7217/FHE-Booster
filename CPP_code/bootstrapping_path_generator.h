#include "shared_utils.h"

#include <iterator>
#include <numeric>
#include <sys/types.h>
#include <sys/stat.h>

class BootstrappingPathGenerator
{
public:
    BootstrappingPathGenerator(OperationList, bool, int);
    std::vector<OperationList> get_bootstrapping_paths(std::string);

private:
    int gained_levels;

    std::vector<std::vector<OperationPtr>> bootstrapping_paths;
    bool using_selective_model;
    OperationList operations;

    void generate_bootstrapping_paths();

    void create_raw_bootstrapping_paths();
    std::vector<std::vector<OperationPtr>> create_bootstrapping_paths_helper(OperationPtr, std::vector<OperationPtr>, int);

    void print_number_of_paths();
    void print_bootstrapping_paths();

    void clean_raw_bootstrapping_paths();
    void remove_last_operation_from_bootstrapping_paths();
    void remove_duplicate_bootstrapping_paths();
    void remove_redundant_bootstrapping_paths();
    bool paths_are_redundant(std::vector<OperationPtr>, std::vector<OperationPtr>);
    void write_paths_to_file(std::ofstream &);
    std::vector<OperationList> read_bootstrapping_paths(std::ifstream &input_file, OperationList);
    void add_path_num_info_to_all_operations();
};