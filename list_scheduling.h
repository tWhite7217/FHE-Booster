#include "LGRParser.h"
#include "DDGs/custom_ddg_format_parser.h"
#include "shared_utils.h"

#include <vector>
#include <map>
#include <queue>
#include <set>
#include <numeric>

class ListScheduler
{
public:
    std::vector<int> schedule;

    ListScheduler(std::string, std::string, int);

    void create_schedule();
    void generate_child_ids();
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
    std::vector<std::vector<int>> bootstrapping_paths;
    LGRParser lgr_parser;

    std::map<int, int> running_operations;
    std::map<int, int> bootstrapping_operations;
    std::vector<int> ordered_unstarted_operations;
    int clock_cycle;

    struct RankCmp
    {
        bool operator()(const int &id_a, const int &id_b) const
        {
            return operations.get(id_a).rank < operations.get(id_b).rank;
        }
    };
    std::set<int, RankCmp> bootstrapping_queue;

    std::function<void()> start_bootstrapping_ready_operations;

    void update_earliest_start_time(Operation &);
    int get_earliest_possible_program_end_time();
    void update_latest_start_time(Operation &, int);
    std::vector<int> get_priority_list();
    std::map<int, int> initialize_pred_count();
    std::set<int> initialize_ready_operations(std::map<int, int>);
    bool operation_is_ready(int, std::map<int, int>, std::vector<int>, std::map<int, int>);
    std::set<int> handle_started_operations(std::map<int, int> &);
    void decrement_cycles_left(std::map<int, int> &);
    std::set<int> get_finished_operations(std::map<int, int> &);
    void start_ready_operations();
    void add_necessary_operations_to_bootstrapping_queue(std::set<int>);
    // void start_bootstrapping_ready_operations();
    void start_bootstrapping_ready_operations_for_unlimited_model();
    void start_bootstrapping_ready_operations_for_limited_model();
    int get_available_bootstrap_core_num();
};