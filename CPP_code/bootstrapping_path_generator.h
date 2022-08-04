#include "shared_utils.h"

#include <iterator>

class BootstrappingPathGenerator
{
public:
    BootstrappingPathGenerator(OperationList, bool);
    std::vector<std::vector<OperationPtr>> generate_bootstrapping_paths();
    std::vector<std::vector<OperationPtr>> generate_bootstrapping_paths_for_validation();

private:
    std::vector<std::vector<OperationPtr>> bootstrapping_paths;
    bool using_selective_model;
    OperationList operations;

    void create_raw_bootstrapping_paths();
    std::vector<std::vector<OperationPtr>> create_bootstrapping_paths_helper(OperationPtr, int, int);

    void create_raw_bootstrapping_paths_for_validation();
    std::vector<std::vector<OperationPtr>> depth_first_search(std::vector<OperationPtr>, std::vector<std::vector<OperationPtr>>);

    void clean_raw_bootstrapping_paths();
    void remove_last_operation_from_bootstrapping_paths();
    void remove_duplicate_bootstrapping_paths();
    void remove_redundant_bootstrapping_paths();
    bool path_is_redundant(size_t);
    bool larger_path_contains_smaller_path(std::vector<OperationPtr>, std::vector<OperationPtr>);
    float get_path_cost(std::vector<OperationPtr>);
};