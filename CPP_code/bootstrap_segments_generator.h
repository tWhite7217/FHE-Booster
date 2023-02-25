#include "shared_utils.h"
#include "program.h"
#include "custom_ddg_format_parser.h"

#include <iterator>
#include <numeric>
#include <sys/types.h>
#include <sys/stat.h>

class BootstrapSegmentGenerator
{
public:
  BootstrapSegmentGenerator(int, char **);
  bool segments_files_are_current() const;
  bool is_in_forced_generation_mode() const;
  void generate_bootstrap_segments();
  void write_segments_to_files();

private:
  const std::string help_info = R"(
Usage: ./bootstrap_segments_generator.out <dag_file> 
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

  Program program;
  std::vector<BootstrapSegment> bootstrap_segments;

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

  struct IdCmp
  {
    bool operator()(const OperationPtr &a, const OperationPtr &b) const
    {
      return a->id < b->id;
    }
  };

  std::map<std::pair<OperationPtr, int>, bool> too_far_from_fresh_ciphertext;

  void find_operations_to_ignore();
  bool is_ignorable(const OperationPtr &) const;

  void create_raw_bootstrap_segments();
  std::vector<BootstrapSegment> create_bootstrap_segments_helper(OperationPtr, BootstrapSegment, int);

  void print_number_of_segments() const;
  void print_bootstrap_segments() const;

  void sort_segments();
  void remove_last_operation_from_segments();
  void remove_redundant_segments();
  bool segments_are_redundant(const BootstrapSegment &, const BootstrapSegment &) const;
  void write_segments_to_file(std::ofstream &) const;
  // void convert_segments_to_standard();
  void convert_segments_to_selective();

  void sort_segments_and_report_time();
  void remove_redundant_segments_and_report_time();

  void parse_args(int, char **);
  void print_options() const;
};