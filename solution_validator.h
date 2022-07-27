#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <functional>

#include "shared_utils.h"
#include "DDGs/custom_ddg_format_parser.h"
#include "LGRParser.h"

class SolutionValidator
{
public:
    SolutionValidator(std::string, std::string);

    void validate_solution();

private:
    using OperationIdToRemainingCyclesMap = std::unordered_map<int, int>;

    std::vector<int> unstarted_operations;
    OperationIdToRemainingCyclesMap running_operations;
    std::vector<int> operations_ready_to_bootstrap;
    OperationIdToRemainingCyclesMap bootstrapping_operations;
    int clock_cycle = -1;
    OperationList operations;

    InputParser input_parser = InputParser(true, false);
    LGRParser lgr_parser;

    void check_bootstrapping_constraints_are_met();
    bool program_is_not_complete();
    OperationIdToRemainingCyclesMap decrement_cycles_left_for(OperationIdToRemainingCyclesMap);
    OperationIdToRemainingCyclesMap remove_finished_operations(OperationIdToRemainingCyclesMap);
    void add_necessary_running_operations_to_bootstrapping_queue();
    void start_bootstrapping_ready_operations_from_queue();
    bool clock_cycle_matches_operation_bootstrapping_start_time(int);
    bool clock_cycle_is_later_than_operation_bootstrapping_start_time(int);
    std::pair<bool, int> check_if_operation_bootstrapping_core_is_available(int);
    void print_bootstrapping_core_is_not_available_error(int, int);
    void print_missed_bootstrapping_deadline_error(int);
    void print_missed_start_time_error(int, int);
    std::vector<int> get_ready_operations();
    bool operation_is_ready(int);
    void start_running_ready_operations(std::vector<int>);
    bool operation_is_waiting_on_parent_to_bootstrap(int, int);
};