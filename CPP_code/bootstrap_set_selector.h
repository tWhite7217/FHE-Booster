#include "program.h"
#include "shared_utils.h"

#include <vector>
#include <map>
#include <queue>
#include <unordered_set>
#include <numeric>

class BootstrapSetSelector
{
public:
  BootstrapSetSelector(int, char **);
  void choose_and_output_bootstrap_sets();

private:
  const std::string help_info = R"(
Usage: ./bootstrap_set_selector <dag_file>
                                <segments_file>
                                <output_file_1>[,<output_file_2>,...,<output_file_n>]
                                <num_levels>
                                [-l <file_1>[,<file_2>,...,<file_n>]]
                                [-s <int_1>[,<int_2>,...,<int_n>]]
                                [-r <int_1>[,<int_2>,...,<int_n>]]
                                [-u <int_1>[,<int_2>,...,<int_n>]]

Arguments:
  <dag_file>
    The text file describing the FHE program as a DAG.
  <segments_file>
    The text file listing the bootstrap segments of the program.
  <output_file>
    The path to the file where the bootstrap set should be saved.
  <num_levels>
    The number of levels between bootstraps, also called the noise
    threshold.
  -l <file>, --latency-file <file>
    The file describing the latencies of FHE operations on the target
    hardware. The default values can be found in program.h.
  Weights:
    The following options apply weights to certain attributes that are
    used in choosing operations to bootstrap. All default to 0.
      -s <int>, --segments-weight=<int>
        This attribute correlates to the number of unsatisfied 
        bootstrap segments containing the operation.
      -r <int>, --slack-weight=<int>
        This attribute correlates to the difference between the
        operation's latest and earliest starting times.
      -u <int>, --urgency-weight=<int>
        This attribute correlates to the current bootstrap
        urgency level of the operation.
        
Batching:
  Notice that multiple output files may be specified as a comma-
  separated list. This is so multiple bootstrap sets can be created
  for a single graph without loading that graph's bootstrap segments
  multiple times. To accomodate this, all used weights must also be
  comma-separated lists of the same length as the list of output files.)";

  struct Options
  {
    std::string dag_filename;
    std::string segments_filename;
    int num_levels;
    std::vector<std::string> output_filenames;
    std::vector<int> segments_weight;
    std::vector<int> slack_weight;
    std::vector<int> urgency_weight;
  } options;

  int max_num_segments;
  int max_slack;

  size_t num_sets;
  size_t set_index = 0;

  Program program;

  void choose_operations_to_bootstrap();

  void choose_operation_to_bootstrap_based_on_score();
  double get_score(const OperationPtr &);
  void parse_args(int, char **);
  void print_options();
};