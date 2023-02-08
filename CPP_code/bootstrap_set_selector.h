#include "custom_ddg_format_parser.h"
#include "shared_utils.h"
#include "bootstrapping_path_generator.h"

#include <vector>
#include <map>
#include <queue>
#include <unordered_set>
#include <numeric>

class BootstrapSetSelector
{
public:
    BootstrapSetSelector(int, char **);
    void choose_and_output_bootstrapping_sets();

private:
    const std::string help_info = R"(
Usage: ./list_scheduler <dag_file> 
                        <output_file_1>[,<output_file_2>,...,<output_file_n>]
                        <num_levels>
                        [-s <int_1>[,<int_2>,...,<int_n>]]
                        [-r <int_1>[,<int_2>,...,<int_n>]]
                        [-u <int_1>[,<int_2>,...,<int_n>]]

Arguments:
  <dag_file>
    The text file describing the FHE program as a DAG.
  <output_file>
    The path to the file where the bootstrapping set should be saved.
  <num_levels>
    The number of levels between bootstraps, also called the noise
    threshold.
  Weights:
    The following options apply weights to certain attributes that are
    used in choosing operations to bootstrap. All default to 0.
      -s <int>, --segments-weight=<int>
        This attribute correlates to the number of unsatisfied 
        bootstrapping segments containing the operation.
      -r <int>, --slack-weight=<int>
        This attribute correlates to the difference between the
        operation's latest and earliest starting times.
      -u <int>, --urgency-weight=<int>
        This attribute correlates to the current bootstrapping
        urgency level of the operation.
        
Batching:
  Notice that multiple output files may be specified as a comma-
  separated list. This is so multiple bootstrapping sets can be created
  for a single graph without loading that graph's bootstrapping segments
  multiple times. To accomodate this, all used weights must also be
  comma-separated lists of the same length as the list of output files.)";

    struct Options
    {
        std::string dag_file_path;
        int num_levels;
        std::vector<std::string> output_file_paths;
        std::vector<int> segments_weight;
        std::vector<int> slack_weight;
        std::vector<int> urgency_weight;
    } options;

    int max_num_paths;
    int max_slack;

    size_t num_sets;
    size_t set_index = 0;

    OperationList operations;
    std::map<std::string, int> operation_type_to_latency_map;
    std::vector<OperationList> bootstrapping_paths;

    void choose_operations_to_bootstrap();
    void write_lgr_like_format();

    void update_all_bootstrap_urgencies();
    int update_num_paths_for_every_operation();
    void choose_operation_to_bootstrap_based_on_score();
    double get_score(OperationPtr);
    void parse_args(int, char **);
    void print_options();
    void reset();
};