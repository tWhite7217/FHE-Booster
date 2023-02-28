#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <queue>
#include <set>
#include <fstream>
#include <cassert>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <array>

#include "../shared_utils.h"
#include "engine_operation.hpp"

enum ExecMode
{
  BOOSTER,
  ALAP,
  PLAINTEXT
};

struct CommandLineOptions
{
  int num_levels = 9;
  double rand_thresh = 1.0;
  ExecMode mode = BOOSTER;
  std::string mode_string = "BOOSTER";
  bool verify_results = false;
  bool bootstrap_inputs = false;
  std::string input_filename;
  std::string eval_time_filename;
  std::string num_bootstraps_filename;
};

struct ScheduleInfo
{
  std::vector<std::vector<EngineOperationPtr>> circuit;
  std::unordered_set<EngineOpInput, EngineOpInput::hash> initial_inputs;
  std::unordered_set<std::string> bootstrap_candidates;
  std::unordered_map<std::string, std::unordered_set<std::string>> dependent_outputs;
};

class ScheduleParser
{
public:
  ScheduleInfo parse(const CommandLineOptions &);

private:
  CommandLineOptions options;
  ScheduleInfo sched_info;
  std::unordered_set<EngineOpInput, EngineOpInput::hash> all_inputs;
  std::set<std::string> outputs;

  void reset_parser(const CommandLineOptions &);
  std::ifstream open_sched_file();
  void parse_line(const std::string &);
  void find_bootstrap_candidates();
  int get_thread_idx(std::string) const;
  std::unordered_set<EngineOpInput, EngineOpInput::hash> get_initial_inputs() const;
};