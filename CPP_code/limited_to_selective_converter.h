#include "custom_ddg_format_parser.h"
#include "LGRParser.h"
#include "bootstrapping_segment_generator.h"

#include <iostream>

class LimitedToSelectiveConverter
{
public:
    LimitedToSelectiveConverter(std::string, std::string, int);
    void remove_unnecessary_bootstrapped_results();
    void write_selective_lgr_file(std::string);

private:
    OperationList operations;
    std::vector<OperationList> bootstrapping_segments;

    bool no_segment_relies_on_parent_child_bootstrapping_pair(OperationPtr &, OperationPtr &);
    bool segment_relies_on_parent_child_bootstrapping_pair(OperationList &, OperationPtr &, OperationPtr &);
};