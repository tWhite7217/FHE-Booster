#include "shared_utils.h"
#include "custom_ddg_format_parser.h"

#include <iterator>
#include <numeric>
#include <sys/types.h>
#include <sys/stat.h>

class BootstrapSegmentGenerator
{
public:
  BootstrapSegmentGenerator(int, char **);
  bool bootstrap_files_are_current();
  bool is_in_forced_generation_mode();
  void generate_bootstrap_segments();
  void write_segments_to_files();

private:
  const std::string help_info = R"(
Usage: ./list_scheduler <dag_file> 
                        <output_file>
                        <num_levels>
                        [-i <initial_levels>]
                        [-F / --force]

Note:
  The output_file argument should not include the file extension.

Arguments:
  <dag_file>
    The text file describing the FHE program as a DAG.
  <output_file>
    The path to the file where the bootstrap set should be saved.
  <num_levels>
    The number of levels between bootstraps, also called the noise
    threshold.
  -i <int>, --initial_levels=<int>
    The number of levels to ignore before generating bootstrap
    segments. Defaults to 0.
  -F, --force
    Forces generation of bootstrap segments, even if the files
    seem current.)";

  int gained_levels;

  std::vector<std::vector<OperationPtr>> bootstrap_segments;
  OperationList operations;

  struct Options
  {
    std::string dag_filename;
    std::string output_filename;
    int num_levels;
    int initial_levels = 0;
    bool force_generation;
  } options;

  std::string executable_filename;
  std::string standard_output_filename;
  std::string selective_output_filename;

  void create_raw_bootstrap_segments();
  std::vector<std::vector<OperationPtr>> create_bootstrap_segments_helper(OperationPtr, std::vector<OperationPtr>, int);

  void print_number_of_segments();
  void print_bootstrap_segments();

  void remove_last_operation_from_bootstrap_segments();
  void remove_redundant_bootstrap_segments();
  bool segments_are_redundant(std::vector<OperationPtr>, std::vector<OperationPtr>);
  void write_segments_to_file(std::ofstream &);
  void convert_segments_to_standard();

  void parse_args(int, char **);
  void print_options();
};