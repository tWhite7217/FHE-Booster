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
    ListScheduler(std::string, std::string, int, int);

    OperationList get_operations();

    void perform_list_scheduling();

    void update_all_ESTs_and_LSTs();
    void update_all_ranks();
    void generate_start_times_and_solver_latency();
    void generate_core_assignments();
    void choose_operations_to_bootstrap();

    void write_lgr_like_format(std::string);
    void write_assembly_like_format(std::string);

private:
    int num_paths_multiplier;
    int rank_multiplier;
    int urgency_multiplier;

    int solver_latency;

    std::string lgr_file_path;

    int num_cores;
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
    void update_num_paths_for_every_operation();
    void initialize_pred_count();
    void update_ready_operations();
    std::unordered_set<OperationPtr> handle_started_operations(std::map<OperationPtr, int> &);
    void decrement_cycles_left(std::map<OperationPtr, int> &);
    std::unordered_set<OperationPtr> get_finished_operations(std::map<OperationPtr, int> &);
    void start_ready_operations();
    void add_necessary_operations_to_bootstrapping_queue(std::unordered_set<OperationPtr>);
    void start_bootstrapping_necessary_operations(std::unordered_set<OperationPtr>);
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
    void mark_cores_available(std::unordered_set<OperationPtr> &);
    void update_pred_count(std::unordered_set<OperationPtr> &, std::unordered_set<OperationPtr> &);
};