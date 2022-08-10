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

    ListScheduler(std::string, std::string, int);

    void create_schedule();
    void update_all_ESTs_and_LSTs();
    void update_all_ranks();
    void generate_start_times_and_solver_latency();
    void write_lgr_like_format(std::string);

private:
    int solver_latency;

    int num_cores;
    std::vector<int> cores;

    OperationList operations;
    std::map<std::string, int> operation_type_to_latency_map;
    bool used_selective_model;
    std::vector<std::vector<OperationPtr>> bootstrapping_paths;
    LGRParser lgr_parser;

    std::map<OperationPtr, int> running_operations;
    std::map<OperationPtr, int> bootstrapping_operations;
    std::vector<OperationPtr> ordered_unstarted_operations;
    int clock_cycle;

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
    std::vector<OperationPtr> get_priority_list();
    std::map<OperationPtr, int> initialize_pred_count();
    std::set<OperationPtr> initialize_ready_operations(std::map<OperationPtr, int>);
    bool operation_is_ready(OperationPtr);
    std::set<OperationPtr> handle_started_operations(std::map<OperationPtr, int> &);
    void decrement_cycles_left(std::map<OperationPtr, int> &);
    std::set<OperationPtr> get_finished_operations(std::map<OperationPtr, int> &);
    void start_ready_operations();
    void add_necessary_operations_to_bootstrapping_queue(std::set<OperationPtr>);
    // void start_bootstrapping_ready_operations();
    void start_bootstrapping_ready_operations_for_unlimited_model();
    void start_bootstrapping_ready_operations_for_limited_model();
    int get_available_bootstrap_core_num();
    bool program_is_not_finished();
};