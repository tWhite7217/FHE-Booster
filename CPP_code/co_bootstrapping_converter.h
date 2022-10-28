#include "custom_ddg_format_parser.h"
#include "LGRParser.h"
#include "bootstrapping_path_generator.h"

#include <iostream>

class CoBootstrappingConverter
{
public:
    CoBootstrappingConverter(std::string, std::string);
    void remove_unnecessary_bootstrapped_results();
    void write_selective_lgr_file(std::string);

private:
    OperationList operations;
    std::vector<OperationList> bootstrapping_paths;

    bool no_path_relies_on_parent_child_bootstrapping_pair(OperationPtr &, OperationPtr &);
    bool path_relies_on_parent_child_bootstrapping_pair(OperationList &, OperationPtr &, OperationPtr &);
};