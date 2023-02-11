#include "LGRParser.h"
#include "program.h"
#include "shared_utils.h"
#include "bootstrap_segments_generator.h"

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

    void write_sched_to_file(const std::string &);
    void write_to_output_files();

private:
    const std::string help_info = R"(
Usage: ./list_scheduler <dag_file>
                        <latency_file>
                        <output_file>
                        [<options>]

Options:
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

    struct SlackCmp
    {
        bool operator()(const OperationPtr &a, const OperationPtr &b) const
        {
            return a->get_slack() < b->get_slack();
        }
    };

    std::map<OperationPtr, int> pred_count;
    std::map<OperationPtr, int> running_operations;
    std::map<OperationPtr, int> bootstrapping_operations;
    std::multiset<OperationPtr, SlackCmp> prioritized_unstarted_operations;
    OpVector ready_operations;
    int clock_cycle;
    int bootstrap_latency;

    std::unordered_set<OperationPtr> finished_running_operations;
    std::unordered_set<OperationPtr> finished_bootstrapping_operations;

    std::multiset<OperationPtr, SlackCmp> bootstrapping_queue;

    std::function<void()> start_bootstrapping_ready_operations;

    void initialize_pred_count();
    void update_ready_operations();
    std::unordered_set<OperationPtr> handle_started_operations(std::map<OperationPtr, int> &);
    void decrement_cycles_left(std::map<OperationPtr, int> &);
    std::unordered_set<OperationPtr> get_finished_operations(std::map<OperationPtr, int> &);
    void start_ready_operations();
    void add_necessary_operations_to_bootstrapping_queue();
    void start_bootstrapping_necessary_operations();
    void start_bootstrapping_ready_operations_for_unlimited_model();
    void start_bootstrapping_ready_operations_for_limited_model();
    int get_best_core_for_operation(OperationPtr, int);
    int get_available_core_num();
    bool core_is_available(int);
    bool program_is_not_finished();
    std::string get_constant_arg(OperationPtr, size_t);
    std::string get_variable_arg(OperationPtr, size_t);
    void mark_cores_available(std::unordered_set<OperationPtr> &);
    void update_pred_count();
    void initialize_simulation_state();
    void update_simulation_state();
    void parse_args(int, char **);
    void print_options();
};