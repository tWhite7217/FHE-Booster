// import sys

#include <iostream>
#include <numeric>
// #include <algorithm>

#include "solution_validator.h"

SolutionValidator::SolutionValidator(std::string dag_file_path, std::string lgr_file_path)
{
    input_parser.parse_input_to_generate_operations(dag_file_path);
    operations = input_parser.get_operations();

    lgr_parser.switchIstream(lgr_file_path);
    lgr_parser.set_operations(operations);
    lgr_parser.lex();

    BootstrappingPathGenerator path_generator(operations, lgr_parser.used_selective_model);
    bootstrapping_paths = path_generator.generate_bootstrapping_paths_for_validation();

    unstarted_operations = operations;
}

void SolutionValidator::validate_solution()
{
    check_bootstrapping_constraints_are_met();

    while (program_is_not_complete())
    {
        clock_cycle += 1;

        bootstrapping_operations = decrement_cycles_left_for(bootstrapping_operations);
        running_operations = decrement_cycles_left_for(running_operations);

        bootstrapping_operations = remove_finished_operations(bootstrapping_operations);
        add_necessary_running_operations_to_bootstrapping_queue();
        running_operations = remove_finished_operations(running_operations);
        start_bootstrapping_ready_operations_from_queue();

        auto ready_operations = get_ready_operations();
        start_running_ready_operations(ready_operations);

        // std::cout << clock_cycle << std::endl;
        // std::cout << "Unstarted: ";
        // for (auto operation_id : unstarted_operations)
        // {
        //     std::cout << operation_id << " ";
        // }
        // std::cout << std::endl;
        // std::cout << "Running: ";
        // for (auto [operation_id, cycles_left] : running_operations)
        // {
        //     std::cout << operation_id << "," << cycles_left << " ";
        // }
        // std::cout << std::endl;
        // std::cout << "Ready to BS: ";
        // for (auto operation_id : operations_ready_to_bootstrap)
        // {
        //     std::cout << operation_id << " ";
        // }
        // std::cout << std::endl;
        // std::cout << "Bootstrapping: ";
        // for (auto [operation_id, cycles_left] : bootstrapping_operations)
        // {
        //     std::cout << operation_id << "," << cycles_left << " ";
        // }
        // std::cout << std::endl;
        // std::cout << std::endl;
    }

    if (clock_cycle != lgr_parser.max_finish_time)
    {
        std::cout << "Error: The latencies mismatch" << std::endl;
        std::cout << "Solver latency: " << lgr_parser.max_finish_time << std::endl;
        std::cout << "Calculated latency: " << clock_cycle << std::endl;
    }
    else
    {
        std::cout << "ILP model results are correct" << std::endl;
    }
}

void SolutionValidator::check_bootstrapping_constraints_are_met()
{
    int unsatisfied_path_index;
    if (lgr_parser.used_selective_model)
    {
        unsatisfied_path_index = find_unsatisfied_bootstrapping_path_index_for_selective_model(bootstrapping_paths);
    }
    else
    {
        unsatisfied_path_index = find_unsatisfied_bootstrapping_path_index(bootstrapping_paths);
    }

    if (unsatisfied_path_index != -1)
    {
        std::cout << "Error: The following path was not satisfied" << std::endl;
        std::cout << "Path: ";
        for (auto operation : bootstrapping_paths[unsatisfied_path_index])
        {
            std::cout << operation->id << ", ";
        }
        std::cout << std::endl;
        exit(0);
    }
}

bool SolutionValidator::program_is_not_complete()
{
    return (unstarted_operations.size() > 0) ||
           (running_operations.size() > 0) ||
           (operations_ready_to_bootstrap.size() > 0) ||
           (bootstrapping_operations.size() > 0);
}

SolutionValidator::OperationPtrToRemainingCyclesMap SolutionValidator::decrement_cycles_left_for(OperationPtrToRemainingCyclesMap operation_map)
{
    for (auto [operation, cycles_left] : operation_map)
    {
        operation_map[operation] = cycles_left - 1;
    }
    return operation_map;
}

SolutionValidator::OperationPtrToRemainingCyclesMap SolutionValidator::remove_finished_operations(OperationPtrToRemainingCyclesMap operation_map)
{
    OperationPtrToRemainingCyclesMap new_operation_map;
    for (auto [operation, cycles_left] : operation_map)
    {
        if (cycles_left > 0)
        {
            new_operation_map[operation] = cycles_left;
        }
    }
    return new_operation_map;
}

void SolutionValidator::add_necessary_running_operations_to_bootstrapping_queue()
{
    for (auto [operation, cycles_left] : running_operations)
    {
        if (cycles_left == 0 && lgr_parser.operation_is_bootstrapped(operation))
        {
            operations_ready_to_bootstrap.push_back(operation);
        }
    }
}

void SolutionValidator::start_bootstrapping_ready_operations_from_queue()
{
    if (lgr_parser.used_bootstrap_limited_model)
    {
        std::vector<OperationPtr> operations_to_remove;
        for (auto operation : operations_ready_to_bootstrap)
        {
            if (clock_cycle_matches_operation_bootstrapping_start_time(operation))
            {
                auto [core_is_available, bootstrapping_operation] =
                    check_if_operation_bootstrapping_core_is_available(operation);
                if (core_is_available)
                {
                    bootstrapping_operations[operation] = bootstrapping_latency;
                    operations_to_remove.push_back(operation);
                }
                else
                {
                    print_bootstrapping_core_is_not_available_error(operation, bootstrapping_operation);
                    exit(0);
                }
            }
            else if (clock_cycle_is_later_than_operation_bootstrapping_start_time(operation))
            {
                print_missed_bootstrapping_deadline_error(operation);
                exit(0);
            }
        }
        for (auto operation_id : operations_to_remove)
        {
            remove_element_from_vector(operations_ready_to_bootstrap, operation_id);
        }
    }
    else
    {
        for (auto operation_id : operations_ready_to_bootstrap)
        {
            bootstrapping_operations[operation_id] = bootstrapping_latency;
        }
        operations_ready_to_bootstrap.clear();
    }
}

bool SolutionValidator::clock_cycle_matches_operation_bootstrapping_start_time(OperationPtr operation)
{
    return clock_cycle == operation->bootstrap_start_time;
}

bool SolutionValidator::clock_cycle_is_later_than_operation_bootstrapping_start_time(OperationPtr operation)
{
    return clock_cycle > operation->bootstrap_start_time;
}

std::pair<bool, OperationPtr> SolutionValidator::check_if_operation_bootstrapping_core_is_available(OperationPtr operation)
{
    for (auto [bootstrapping_operation, _] : bootstrapping_operations)
    {
        if (operations_bootstrap_on_same_core(operation, bootstrapping_operation))
        {
            return std::pair{false, bootstrapping_operation};
        }
    }
    return std::pair{true, nullptr};
}

void SolutionValidator::print_bootstrapping_core_is_not_available_error(OperationPtr operation, OperationPtr bootstrapping_operation)
{
    std::cout << "Error: Bootstrapping operation " << bootstrapping_operation->id << " was not completed before operation " << operation->id << " started bootstrapping on core " << operation->core_num << std::endl;
}

void SolutionValidator::print_missed_bootstrapping_deadline_error(OperationPtr operation)
{
    std::cout << "Error: Bootstrapping operation " << operation->id << " did not start on time" << std::endl;
}

void SolutionValidator::print_missed_start_time_error(OperationPtr operation, int clock_cycle)
{
    std::cout << "Error: Operation " << operation->id << " did not start on time" << std::endl;
    std::cout << "Expected start time " << operation->start_time << ". Actual start time " << clock_cycle << std::endl;
}

std::vector<OperationPtr> SolutionValidator::get_ready_operations()
{
    std::vector<OperationPtr> ready_operations;
    for (auto operation : unstarted_operations)
    {
        if (operation_is_ready(operation))
        {
            ready_operations.push_back(operation);
        }
    }
    return ready_operations;
}

bool SolutionValidator::operation_is_ready(OperationPtr operation)
{
    for (auto parent : operation->parent_ptrs)
        if (
            vector_contains_element(unstarted_operations, parent) ||
            unordered_map_contains_key(running_operations, parent) ||
            operation_is_waiting_on_parent_to_bootstrap(operation, parent))
        {
            return false;
        }
    return true;
}

bool SolutionValidator::operation_is_waiting_on_parent_to_bootstrap(OperationPtr operation_id, OperationPtr parent_id)
{
    return lgr_parser.operation_is_bootstrapped(parent_id, operation_id) &&
           (vector_contains_element(operations_ready_to_bootstrap, parent_id) ||
            unordered_map_contains_key(bootstrapping_operations, parent_id));
}

void SolutionValidator::start_running_ready_operations(std::vector<OperationPtr> ready_operations)
{
    for (auto operation : ready_operations)
    {
        if (operation->start_time == clock_cycle)
        {
            running_operations[operation] = input_parser.get_operation_type_to_latency_map()[operation->type];
            remove_element_from_vector(unstarted_operations, operation);
        }
        else if (operation->start_time < clock_cycle)
        {
            print_missed_start_time_error(operation, clock_cycle);
            exit(0);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Two files must be provided in the command line.\n";
        return 0;
    }

    std::string dag_file_path = std::string{argv[1]};
    std::string lgr_file_path = std::string{argv[2]};

    auto solution_validator = SolutionValidator(dag_file_path, lgr_file_path);
    solution_validator.validate_solution();
    return 0;
}
