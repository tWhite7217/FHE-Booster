// bootstrapping_latency = 12

#pragma once

#include <vector>
#include <map>
#include <unordered_map>

#include "shared_utils.h"

class SolutionValidator
{
public:
    struct SolutionValidatorConfig
    {
        int bootstrapping_latency;
        std::vector<Operation> operations;
        std::map<std::string, int> operation_type_to_latency_map;
        std::vector<std::vector<int>> bootstrapping_paths;
        int solver_latency;
        std::vector<int> bootstrapped_operation_ids;
        bool used_bootstrap_limited_model;
        std::vector<int> cores_used_to_bootstrap;
        bool used_some_children_model;
    };

    SolutionValidator(SolutionValidatorConfig config);

    void validate_solution();

private:
    using OperationIdToRemainingCyclesMap = std::unordered_map<int, int>;

    SolutionValidatorConfig config;
    std::vector<int> unstarted_operations;
    OperationIdToRemainingCyclesMap running_operations;
    std::vector<int> operations_ready_to_bootstrap;
    OperationIdToRemainingCyclesMap bootstrapping_operations;
    int clock_cycle = -1;

    void check_bootstrapping_constraints_are_met();
    bool program_is_not_complete();
    OperationIdToRemainingCyclesMap decrement_cycles_left_for(OperationIdToRemainingCyclesMap);
    OperationIdToRemainingCyclesMap remove_finished_operations(OperationIdToRemainingCyclesMap);
    void add_necessary_running_operations_to_bootstrapping_queue();
    void start_bootstrapping_ready_operations_from_queue();
    bool clock_cycle_matches_operation_bootstrapping_start_time(int);
    bool clock_cycle_is_later_than_operation_bootstrapping_start_time(int);
    std::pair<bool, int> check_if_operation_bootstrapping_core_is_available(int);
    bool operations_bootstrap_on_same_core(int, int);
    void print_bootstrapping_core_is_not_available_error(int, int);
    void print_missed_bootstrapping_deadline_error(int);
    void print_missed_start_time_error(int);
    std::vector<int> get_ready_operations();
    bool operation_is_ready(int);
    void start_running_ready_operations(std::vector<int>);
    Operation get_operation_from_id(int id);
    bool operation_is_waiting_on_parent_to_bootstrap(int, int);
    bool parent_sends_bootstrapped_result_to_child(int, int);
    bool operation_is_bootstrapped(int);
};