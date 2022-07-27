#include "LGRParser.h"
#include "shared_utils.h"

#include <vector>
#include <map>
#include <queue>
#include <set>

class ListScheduler
{
public:
    std::vector<int> schedule;

    ListScheduler(std::string, std::string);

    void create_schedule();
    void generate_child_ids();
    void update_all_ESTs_and_LSTs();
    void update_all_ranks();
    void generate_start_times_and_solver_latency(int);
    void write_lgr_like_format(std::string);

private:
    int solver_latency;

    OperationList operations;
    std::map<std::string, int> operation_type_to_latency_map;
    bool used_selective_model;
    std::vector<std::vector<int>> bootstrapping_paths;
    LGRParser lgr_parser;

    void update_earliest_start_time(Operation &);
    int get_earliest_possible_program_end_time();
    void update_latest_start_time(Operation &, int);
    std::vector<int> get_priority_list();
    std::map<int, int> initialize_pred_count();
    std::set<int> initialize_ready_operations(std::map<int, int>);
    bool operation_is_ready(int, std::map<int, int>, std::vector<int>, std::map<int, int>);
};