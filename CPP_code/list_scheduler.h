#include "LGRParser.h"
#include "custom_ddg_format_parser.h"
#include "shared_utils.h"
#include "bootstrapping_path_generator.h"

#include <vector>
#include <map>
#include <queue>
#include <unordered_set>
#include <numeric>

class ListScheduler
{
public:
    ListScheduler(int, char **);

    OperationList get_operations();

    void perform_list_scheduling();

    void update_all_ESTs_and_LSTs();
    int update_all_ranks();
    void generate_start_times_and_solver_latency();
    void generate_core_assignments();
    void choose_operations_to_bootstrap();

    void write_lgr_like_format();
    void write_assembly_like_format();

private:
    const std::string help_info = R"(
Usage: ./list_scheduler <dag_file> <output_file> [<options>]

Options:
  -t <int>, --num-threads=<int>
    The number of threads on which operations may be scheduled.
  -n, --no-bootstrap
    Creates a schedule without bootstrapping.
  -i <file>, --input-lgr=<file>
    A path to a .lgr file specifying a set of operations to bootstrap.
    Ignored if -n/--no-bootstrap is used.
  -l <int>, --num-levels=<int>
    The number of levels between bootstraps, also called the noise
    threshold. Defaults to 9. Ignored if the -n/--no-bootstrap option
    or the -i/--input-lgr option is used.
  Weights:
    The following options apply weights to certain attributes that are
    used in choosing operations to bootstrap. These options are
    ignored if the -n/--no-bootstrap option or the -i/--input-lgr
    option is used. All default to 0.
      -s <int>, --segments-weight=<int>
        This attribute correlates to the number of unsatisfied 
        bootstrapping segments containing the operation.
      -r <int>, --slack-weight=<int>
        This attribute correlates to the difference between the
        operation's latest and earliest starting times.
      -u <int>, --urgency-weight=<int>
        This attribute correlates to the current bootstrapping
        urgency level of the operation.)";

    struct Options
    {
        std::string dag_file_path;
        std::string output_file_path;
        std::string lgr_file_path;
        bool no_bootstrapping = false;
        int num_threads = 1;
        int num_levels = 9;
        int segments_weight = 0;
        int slack_weight = 0;
        int urgency_weight = 0;
    } options;

    int max_num_paths;
    int max_slack;

    int solver_latency;

    std::unordered_map<int, bool> core_availability;

    bool create_core_assignments;
    std::vector<std::string> core_schedules;

    OperationList operations;
    std::map<std::string, int> operation_type_to_latency_map;
    std::vector<OperationList> bootstrapping_paths;
    LGRParser lgr_parser;

    struct PriorityCmp
    {
        bool operator()(const OperationPtr &a, const OperationPtr &b) const
        {
            return a->rank < b->rank;
        }
    };

    std::map<OperationPtr, int> pred_count;
    std::map<OperationPtr, int> running_operations;
    std::map<OperationPtr, int> bootstrapping_operations;
    std::multiset<OperationPtr, PriorityCmp> prioritized_unstarted_operations;
    OperationList ready_operations;
    int clock_cycle;

    std::unordered_set<OperationPtr> finished_running_operations;
    std::unordered_set<OperationPtr> finished_bootstrapping_operations;

    struct RankCmp
    {
        bool operator()(const OperationPtr &a, const OperationPtr &b) const
        {
            return a->rank < b->rank;
        }
    };
    std::multiset<OperationPtr, RankCmp> bootstrapping_queue;

    std::function<void()> start_bootstrapping_ready_operations;

    void update_earliest_start_time(OperationPtr);
    int get_earliest_possible_program_end_time();
    void update_latest_start_time(OperationPtr, int);
    void update_all_bootstrap_urgencies();
    int update_num_paths_for_every_operation();
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
    void choose_operation_to_bootstrap_based_on_score();
    double get_score(OperationPtr);
    std::string get_constant_arg(OperationPtr, size_t);
    std::string get_variable_arg(OperationPtr, size_t);
    void mark_cores_available(std::unordered_set<OperationPtr> &);
    void update_pred_count();
    void initialize_simulation_state();
    void update_simulation_state();
    void parse_args(int, char **);
    void print_options();
};