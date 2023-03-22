#include "shared_utils.h"
#include "program.h"
#include "file_writer.h"

#include <vector>
#include <map>
#include <queue>
#include <unordered_set>
#include <numeric>

class ListScheduler
{
public:
  ListScheduler(int, char **);

  void perform_list_scheduling();

  void generate_start_times_and_solver_latency();
  void generate_core_assignments();

  void write_sched_to_file(const std::string &) const;
  void write_to_output_files() const;

  std::string get_log_filename() const;

private:
  const std::string help_info = R"(
Usage: ./list_scheduler.out <dag_file>
                            <output_file>
                            [<options>]

Options:
  -l <file>, --latency-file=<file>
    A file describing the latencies of FHE operations on the target
    hardware. The default values can be found in program.h.
  -t <int>, --num-threads=<int>
    The number of threads on which operations may be scheduled.
    Defaults to 1.
  -i <file/"NULL">, --input-lgr=<file/"NULL">
    A path to a .lgr file specifying a set of operations to bootstrap.
    Setting to "NULL" means scheduling will be performed without
    bootstrapping. Defaults to "NULL".)";

  struct Options
  {
    std::string dag_filename;
    std::string latency_filename;
    std::string output_filename;
    std::string bootstrap_filename = "NULL";
    int num_threads = 1;
  } options;

  int solver_latency;

  std::unordered_map<int, bool> core_availability;

  bool create_core_assignments;
  std::vector<std::string> core_schedules;

  Program program;

  struct PriorityCmp
  {
    bool operator()(const OperationPtr &a, const OperationPtr &b) const
    {
      auto a_slack = a->get_slack();
      auto b_slack = b->get_slack();
      if (a_slack == b_slack)
      {
        return a->id < b->id;
      }
      else
      {
        return a_slack < b_slack;
      }
    }
  };

  std::map<OperationPtr, int> pred_count;
  std::map<OperationPtr, int> running_operations;
  std::map<OperationPtr, int> bootstrapping_operations;
  std::set<OperationPtr, PriorityCmp> prioritized_unstarted_operations;
  OpVector ready_operations;
  int clock_cycle;
  int bootstrap_latency;

  OpSet finished_running_operations;
  OpSet finished_bootstrapping_operations;

  std::set<OperationPtr, PriorityCmp> bootstrapping_queue;

  std::function<void()> start_bootstrapping_ready_operations;

  void initialize_pred_count();
  void update_ready_operations();
  OpSet handle_started_operations(std::map<OperationPtr, int> &);
  void decrement_cycles_left(std::map<OperationPtr, int> &);
  OpSet get_finished_operations(std::map<OperationPtr, int> &);
  void start_ready_operations();
  void add_necessary_operations_to_bootstrapping_queue();
  void start_bootstrapping_necessary_operations();
  void start_bootstrapping_ready_operations_for_unlimited_model();
  void start_bootstrapping_ready_operations_for_limited_model();
  void mark_cores_available(const OpSet &);
  void update_pred_count();
  void initialize_simulation_state();
  void update_simulation_state();
  void parse_args(int, char **);

  int get_best_core_for_operation(const OperationPtr &, int) const;
  int get_available_core_num() const;
  bool core_is_available(int) const;
  bool program_is_not_finished() const;
  std::string get_constant_arg(const OperationPtr &, size_t) const;
  std::string get_variable_arg(const OperationPtr &, size_t) const;
  void print_options() const;
};