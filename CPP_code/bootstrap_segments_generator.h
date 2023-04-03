#include "shared_utils.h"
#include "program.h"
#include "file_writer.h"

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
  std::string get_log_filename() const;

private:
  static const std::string help_info;

  Program program;
  std::vector<BootstrapSegment> bootstrap_segments;

  std::vector<BootstrapSegment> removed_segments;

  std::map<int, std::map<OperationPtr, std::vector<BootstrapSegment>>> back_segs;

  struct Options
  {
    std::string executable_filename;
    std::string dag_filename;
    std::string output_filename;
    int num_levels;
    int initial_levels = 0;
    bool write_text_files;
    bool force_generation;
  } options;

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
  void get_segs_from_children(const OperationPtr &, const int);
  void clean_children_memory(const OperationPtr, const int);
  bool clean_child_memory(const OperationPtr, const int, const int);

  void print_bootstrap_segments() const;

  void sort_segments();
  void remove_redundant_segments();
  bool segments_are_redundant(const BootstrapSegment &, const BootstrapSegment &) const;

  void write_files(const std::string &);

  void sort_segments_and_report_time();
  void remove_redundant_segments_and_report_time();

  void reinstate_removed_redundant_segments();
  void convert_segments_to_selective();

  void parse_args(int, char **);
  void print_options() const;
};

const std::string BootstrapSegmentGenerator::help_info = R"(
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
  -n, --no-text-files
    Disables the creation of human-readable text files that are,
    by default, created in addition to the binary files.
  -i <int>, --initial_levels=<int>
    The number of levels to ignore before generating bootstrap
    segments. Defaults to 0.
  -F, --force
    Forces generation of bootstrap segments, even if the files
    seem current. Can be useful to update or create text files
    even when other files are up to date.)";