#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <functional>

#include "shared_utils.h"
#include "DDGs/custom_ddg_format_parser.h"
#include "LGRParser.h"
#include "bootstrapping_path_generator.h"

class SolutionValidator
{
public:
    SolutionValidator(std::string, std::string);

    void validate_solution();

private:
    using OperationPtrToRemainingCyclesMap = std::unordered_map<OperationPtr, int>;

    std::vector<OperationPtr> unstarted_operations;
    OperationPtrToRemainingCyclesMap running_operations;
    std::vector<OperationPtr> operations_ready_to_bootstrap;
    OperationPtrToRemainingCyclesMap bootstrapping_operations;
    int clock_cycle = -1;
    OperationList operations;

    InputParser input_parser;
    LGRParser lgr_parser;
    std::vector<OperationList> bootstrapping_paths;

    void check_bootstrapping_constraints_are_met();
    bool program_is_not_complete();
    OperationPtrToRemainingCyclesMap decrement_cycles_left_for(OperationPtrToRemainingCyclesMap);
    OperationPtrToRemainingCyclesMap remove_finished_operations(OperationPtrToRemainingCyclesMap);
    void add_necessary_running_operations_to_bootstrapping_queue();
    void start_bootstrapping_ready_operations_from_queue();
    bool clock_cycle_matches_operation_bootstrapping_start_time(OperationPtr);
    bool clock_cycle_is_later_than_operation_bootstrapping_start_time(OperationPtr);
    std::pair<bool, OperationPtr> check_if_operation_bootstrapping_core_is_available(OperationPtr);
    void print_bootstrapping_core_is_not_available_error(OperationPtr, OperationPtr);
    void print_missed_bootstrapping_deadline_error(OperationPtr);
    void print_missed_start_time_error(OperationPtr, int);
    std::vector<OperationPtr> get_ready_operations();
    bool operation_is_ready(OperationPtr);
    void start_running_ready_operations(std::vector<OperationPtr>);
    bool operation_is_waiting_on_parent_to_bootstrap(OperationPtr, OperationPtr);
};