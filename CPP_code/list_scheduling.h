#include "LGRParser.h"
#include "custom_ddg_format_parser.h"
#include "shared_utils.h"
#include "bootstrapping_path_generator.h"

#include <vector>
#include <map>
#include <queue>
#include <set>
#include <numeric>

class ListScheduler
{
public:
    std::vector<OperationPtr> schedule;

    ListScheduler(std::string, std::string, int, bool);

    OperationList get_operations();

    void perform_list_scheduling();

    void create_schedule();
    void update_all_ESTs_and_LSTs();
    void update_all_ranks();
    void generate_start_times_and_solver_latency();
    void generate_core_assignments();
    void choose_operations_to_bootstrap();

    void write_lgr_like_format(std::string);
    void write_assembly_like_format(std::string);

private:
    // use for num_paths with slack heuristic
    // const int num_paths_multiplier = 12;
    // const int rank_multiplier = 2;

    // use for num_paths minus slack heuristic
    const int num_paths_multiplier = 25;
    const int rank_multiplier = -1;

    // use for urgency and num_paths heuristic
    // const int num_paths_multiplier = 5;
    // const int urgency_multiplier = 10;

    int solver_latency;

    std::string lgr_file_path;

    int num_cores;
    std::vector<int> cores;

    bool create_core_assignments;
    std::vector<std::string> core_schedules;

    OperationList operations;
    std::map<std::string, int> operation_type_to_latency_map;
    std::vector<std::vector<OperationPtr>> bootstrapping_paths;
    LGRParser lgr_parser;

    std::map<OperationPtr, int> running_operations;
    std::map<OperationPtr, int> bootstrapping_operations;
    std::vector<OperationPtr> ordered_unstarted_operations;
    int clock_cycle;

    int constant_counter = 0;

    struct RankCmp
    {
        bool operator()(const OperationPtr &a, const OperationPtr &b) const
        {
            return a->rank < b->rank;
        }
    };
    std::multiset<OperationPtr, RankCmp> bootstrapping_queue;

    std::function<void()> start_bootstrapping_ready_operations;

    std::vector<OperationPtr> get_operations_in_topological_order();
    void update_earliest_start_time(OperationPtr);
    int get_earliest_possible_program_end_time();
    void update_latest_start_time(OperationPtr, int);
    void update_all_bootstrap_urgencies();
    std::vector<OperationPtr> get_priority_list();
    std::map<OperationPtr, int> initialize_pred_count();
    std::set<OperationPtr> initialize_ready_operations(std::map<OperationPtr, int>);
    bool operation_is_ready(OperationPtr);
    std::set<OperationPtr> handle_started_operations(std::map<OperationPtr, int> &);
    void decrement_cycles_left(std::map<OperationPtr, int> &);
    std::set<OperationPtr> get_finished_operations(std::map<OperationPtr, int> &);
    void start_ready_operations();
    void add_necessary_operations_to_bootstrapping_queue(std::set<OperationPtr>);
    void start_bootstrapping_necessary_operations(std::set<OperationPtr>);
    bool later_operation_exceeds_urgency_threshold(OperationPtr &, float &);
    void start_bootstrapping_ready_operations_for_unlimited_model();
    void start_bootstrapping_ready_operations_for_limited_model();
    int get_best_core_for_operation(OperationPtr, int);
    int get_available_core_num();
    bool core_is_available(int);
    bool program_is_not_finished();
    void choose_operation_to_bootstrap_based_on_score();
    int get_score(OperationPtr);
    int get_num_bootstrapping_paths_containing_operation(OperationPtr);
    std::string get_constant_arg();
    std::string get_variable_arg(OperationPtr, int);
};