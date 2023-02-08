#include "shared_utils.h"

#include <iterator>
#include <numeric>
#include <sys/types.h>
#include <sys/stat.h>

// options.initial_levels = get_list_arg(options_string, "-i", "--initial_levels", help_info, num_sets, 0, stoi_function);

class BootstrappingPathGenerator
{
public:
  BootstrappingPathGenerator(int, char **);
  bool bootstrapping_files_are_current();
  void generate_bootstrapping_paths();
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
    The path to the file where the bootstrapping set should be saved.
  <num_levels>
    The number of levels between bootstraps, also called the noise
    threshold.
  -i <int>, --initial_levels=<int>
    The number of levels to ignore before generating bootstrapping
    segments. Defaults to 0.
  -F, --force
    Forces generation of bootstrapping segments, even if the files
    seem current.)";

  int gained_levels;

  std::vector<std::vector<OperationPtr>> bootstrapping_paths;
  OperationList operations;

  struct Options
  {
    std::string dag_file_path;
    std::string output_file_path;
    int num_levels;
    int initial_levels = 0;
  } options;

  std::string executable_file;
  std::string standard_output_file_path;
  std::string selective_output_file_path;

  void create_raw_bootstrapping_paths();
  std::vector<std::vector<OperationPtr>> create_bootstrapping_paths_helper(OperationPtr, std::vector<OperationPtr>, int);

  void print_number_of_paths();
  void print_bootstrapping_paths();

  void remove_last_operation_from_bootstrapping_paths();
  void remove_redundant_bootstrapping_paths();
  bool paths_are_redundant(std::vector<OperationPtr>, std::vector<OperationPtr>);
  void write_segments_to_file(std::ofstream &);
  void convert_segments_to_standard();

  void parse_args(int, char **);
  void print_options();
};