#include "list_scheduling.h"

ListScheduler::ListScheduler(std::string dag_file_path, std::string lgr_file_path, int num_cores, bool create_core_assignments)
    : lgr_file_path{lgr_file_path}, num_cores{num_cores}, create_core_assignments{create_core_assignments}
{
    InputParser input_parser;
    input_parser.parse_input_to_generate_operations(dag_file_path);
    operations = input_parser.get_operations();
    operation_type_to_latency_map = input_parser.get_operation_type_to_latency_map();

    if (lgr_file_path != "NULL")
    {
        lgr_parser.switchIstream(lgr_file_path);
        lgr_parser.set_operations(operations);
        lgr_parser.lex();
    }

    BootstrappingPathGenerator path_generator(operations, lgr_parser.used_selective_model);
    bootstrapping_paths = path_generator.generate_bootstrapping_paths();

    if (num_cores > 0)
    {
        cores.resize(num_cores);
        core_schedules.resize(num_cores);
        std::iota(cores.begin(), cores.end(), 1);
        start_bootstrapping_ready_operations = [this]()
        { start_bootstrapping_ready_operations_for_limited_model(); };
    }
    else
    {
        start_bootstrapping_ready_operations = [this]()
        { start_bootstrapping_ready_operations_for_unlimited_model(); };
    }
}

OperationList ListScheduler::get_operations()
{
    return operations;
}

void ListScheduler::update_earliest_start_time(OperationPtr operation)
{
    for (auto parent : operation->parent_ptrs)
    {
        int parent_latency = operation_type_to_latency_map[parent->type];
        if (vector_contains_element(parent->child_ptrs_that_receive_bootstrapped_result, operation))
        {
            parent_latency += bootstrapping_latency;
        }
        int possible_est =
            parent->earliest_start_time +
            parent_latency;
        if (possible_est > operation->earliest_start_time)
        {
            operation->earliest_start_time = possible_est;
        }
    }
}

int ListScheduler::get_earliest_possible_program_end_time()
{
    int earliest_possible_program_end_time = 0;
    for (auto operation : operations)
    {
        auto latency = operation_type_to_latency_map[operation->type];
        int operation_earliest_end_time = operation->earliest_start_time + latency;
        if (operation_earliest_end_time > earliest_possible_program_end_time)
        {
            earliest_possible_program_end_time = operation_earliest_end_time;
        }
    }
    return earliest_possible_program_end_time;
}

void ListScheduler::update_latest_start_time(OperationPtr operation, int earliest_possible_program_end_time)
{
    operation->latest_start_time = std::numeric_limits<decltype(operation->latest_start_time)>::max();

    if (operation->child_ptrs.empty())
    {
        operation->latest_start_time =
            earliest_possible_program_end_time - operation_type_to_latency_map[operation->type];
    }
    else
    {
        for (auto child : operation->child_ptrs)
        {
            int child_latency = operation_type_to_latency_map[child->type];
            if (operation_is_bootstrapped(child))
            {
                child_latency += bootstrapping_latency;
            }
            int possible_lst =
                child->latest_start_time -
                child_latency;
            if (possible_lst < operation->latest_start_time)
            {
                operation->latest_start_time = possible_lst;
            }
        }
    }
}

void ListScheduler::update_all_ESTs_and_LSTs()
{
    // auto operations_in_topological_order = get_operations_in_topological_order();
    // TGFF puts operations in topological order so we do not need to do that here
    auto operations_in_topological_order = operations;

    for (auto operation : operations_in_topological_order)
    {
        update_earliest_start_time(operation);
        // std::cout << "EST of " << operation->id << " is " << operation->earliest_start_time << std::endl;
    }

    int earliest_program_end_time = get_earliest_possible_program_end_time();

    auto operations_in_reverse_topological_order = operations_in_topological_order;
    std::reverse(operations_in_reverse_topological_order.begin(), operations_in_reverse_topological_order.end());

    for (auto operation : operations_in_reverse_topological_order)
    {
        update_latest_start_time(operation, earliest_program_end_time);
        // std::cout << "LST of " << operation->id << " is " << operation->latest_start_time << std::endl;
    }
}

std::vector<OperationPtr> ListScheduler::get_operations_in_topological_order()
{
    std::vector<OperationPtr> unvisited_operations = operations;
    std::vector<OperationPtr> operations_in_topological_order;

    while (!unvisited_operations.empty())
    {
        auto operation_it = unvisited_operations.begin();
        while (operation_it != unvisited_operations.end())
        {
            auto operation = *operation_it;
            bool operation_is_ready = true;

            for (auto potential_parent_operation : unvisited_operations)
            {
                if (vector_contains_element(operation->parent_ptrs, potential_parent_operation))
                {
                    operation_is_ready = false;
                    break;
                }
            }

            if (operation_is_ready)
            {
                operations_in_topological_order.push_back(operation);
                operation_it = unvisited_operations.erase(operation_it);
            }
            else
            {
                operation_it++;
            }
        }
    }

    return operations_in_topological_order;
}

void ListScheduler::update_all_ranks()
{
    for (auto &operation : operations)
    {
        operation->rank = operation->latest_start_time - operation->earliest_start_time;
    }
}

std::vector<OperationPtr> ListScheduler::get_priority_list()
{
    auto priority_list = operations;

    std::sort(priority_list.begin(), priority_list.end(),
              [this](OperationPtr a, OperationPtr b)
              {
                  return a->rank > b->rank;
              });
    return priority_list;
}

std::map<OperationPtr, int> ListScheduler::initialize_pred_count()
{
    std::map<OperationPtr, int> pred_count;
    for (auto operation : operations)
    {
        pred_count[operation] = operation->parent_ptrs.size();
    }
    return pred_count;
}

std::set<OperationPtr> ListScheduler::initialize_ready_operations(std::map<OperationPtr, int> pred_count)
{
    std::set<OperationPtr> ready_operations;
    for (const auto [id, count] : pred_count)
    {
        if (count == 0)
        {
            ready_operations.insert(id);
        }
    }
    return ready_operations;
}

void ListScheduler::create_schedule()
{
    auto priority_list = get_priority_list();
    auto pred_count = initialize_pred_count();
    auto ready_operations = initialize_ready_operations(pred_count);

    while (!ready_operations.empty())
    {
        for (const auto operation : priority_list)
        {
            if (set_contains_element(ready_operations, operation))
            {
                schedule.push_back(operation);
                ready_operations.erase(operation);

                for (auto child : operation->child_ptrs)
                {
                    pred_count[child]--;
                    if (pred_count[child] == 0)
                    {
                        ready_operations.insert(child);
                    }
                }
            }
        }
    }
}

bool ListScheduler::operation_is_ready(OperationPtr operation)
{
    for (auto parent : operation->parent_ptrs)
    {
        if (map_contains_key(running_operations, parent) ||
            vector_contains_element(ordered_unstarted_operations, parent) ||
            (vector_contains_element(parent->child_ptrs_that_receive_bootstrapped_result, operation) &&
             (multiset_contains_element(bootstrapping_queue, parent) ||
              map_contains_key(bootstrapping_operations, parent))))
        {
            return false;
        }
    }
    return true;
}

void ListScheduler::generate_start_times_and_solver_latency()
{
    clock_cycle = 0;
    running_operations.clear();
    bootstrapping_operations.clear();
    bootstrapping_queue.clear();
    ordered_unstarted_operations = schedule;

    while (program_is_not_finished())
    {
        start_ready_operations();

        clock_cycle++;

        handle_started_operations(bootstrapping_operations);

        auto finished_running_ids = handle_started_operations(running_operations);
        add_necessary_operations_to_bootstrapping_queue(finished_running_ids);
        start_bootstrapping_ready_operations();
    }
    // clock_cycle += map_max_value(running_operations);
    solver_latency = clock_cycle;
}

void ListScheduler::generate_core_assignments()
{
    clock_cycle = 0;
    running_operations.clear();
    bootstrapping_operations.clear();
    ordered_unstarted_operations = schedule;

    while (program_is_not_finished())
    {
        start_ready_operations();

        clock_cycle++;

        handle_started_operations(bootstrapping_operations);

        auto finished_running_ids = handle_started_operations(running_operations);
        start_bootstrapping_necessary_operations(finished_running_ids);
    }
    // clock_cycle += map_max_value(running_operations);
    solver_latency = clock_cycle;
}

bool ListScheduler::program_is_not_finished()
{
    return !ordered_unstarted_operations.empty() ||
           !bootstrapping_queue.empty() ||
           !bootstrapping_operations.empty() ||
           !running_operations.empty();
}

std::set<OperationPtr> ListScheduler::handle_started_operations(std::map<OperationPtr, int> &started_operations)
{
    decrement_cycles_left(started_operations);
    auto finished_operations = get_finished_operations(started_operations);
    remove_key_subset_from_map(started_operations, finished_operations);
    return finished_operations;
}

void ListScheduler::start_ready_operations()
{
    std::set<OperationPtr> started_operation_ids;

    for (auto &operation : ordered_unstarted_operations)
    {
        int available_core = get_available_core_num();
        if (operation_is_ready(operation) && (!create_core_assignments || available_core != -1))
        {
            operation->start_time = clock_cycle;
            running_operations[operation] = operation_type_to_latency_map[operation->type];
            started_operation_ids.insert(operation);
            if (create_core_assignments)
            {
                int best_core = get_best_core_for_operation(operation, available_core);
                operation->core_num = best_core;

                // for (auto parent : operation->parent_ptrs)
                // {
                //     if (parent->core_num != best_core)
                //     {
                //         std::string wait_line = "wait " + std::to_string(parent->id) + "\n";
                //         core_schedules[best_core] += wait_line;
                //     }
                // }
                std::string result_var = " c" + std::to_string(operation->id);

                std::string arg1;
                std::string arg2;

                switch (operation->parent_ptrs.size())
                {
                case 0:
                    arg1 = get_constant_arg();
                    arg2 = get_constant_arg();
                    break;
                case 1:
                    arg1 = get_variable_arg(operation, 0);
                    arg2 = get_constant_arg();
                    break;
                case 2:
                    arg1 = get_variable_arg(operation, 0);
                    arg2 = get_variable_arg(operation, 1);
                    break;
                }

                std::string thread = " t" + std::to_string(best_core);

                core_schedules[best_core] += operation->type + result_var + arg1 + arg2 + thread + "\n";
                if (operation_is_bootstrapped(operation))
                {
                    core_schedules[best_core] += "BOOT c0" + std::to_string(operation->id) + " c" + std::to_string(operation->id) + " t" + std::to_string(best_core) + "\n";
                }
            }
        }
    }

    remove_element_subset_from_vector(ordered_unstarted_operations, started_operation_ids);
}

std::string ListScheduler::get_constant_arg()
{
    return " k" + std::to_string(constant_counter++);
}

std::string ListScheduler::get_variable_arg(OperationPtr operation, int parent_num)
{
    std::string arg = " c";
    if (vector_contains_element(operation->parent_ptrs[parent_num]->child_ptrs_that_receive_bootstrapped_result, operation))
    {
        arg += "0";
    }
    return arg + std::to_string(operation->parent_ptrs[parent_num]->id);
}

int ListScheduler::get_best_core_for_operation(OperationPtr operation, int fallback_core)
{
    int best_core = fallback_core;
    for (auto parent : operation->parent_ptrs)
    {
        if (core_is_available(parent->core_num))
        {
            best_core = parent->core_num;
        }
    }
    return best_core;
}

void ListScheduler::decrement_cycles_left(std::map<OperationPtr, int> &started_operations)
{
    for (auto &[operation, time_left] : started_operations)
    {
        time_left--;
    }
}

std::set<OperationPtr> ListScheduler::get_finished_operations(std::map<OperationPtr, int> &started_operations)
{
    std::set<OperationPtr> finished_operations;

    for (auto &[operation, time_left] : started_operations)
    {
        if (time_left == 0)
        {
            finished_operations.insert(operation);
        }
    }

    return finished_operations;
}

void ListScheduler::add_necessary_operations_to_bootstrapping_queue(std::set<OperationPtr> finished_running_operations)
{
    for (auto operation : finished_running_operations)
    {
        if (operation_is_bootstrapped(operation))
        {
            bootstrapping_queue.insert(operation);
        }
    }
}

void ListScheduler::start_bootstrapping_necessary_operations(std::set<OperationPtr> finished_running_operations)
{
    for (auto operation : finished_running_operations)
    {
        if (operation_is_bootstrapped(operation))
        {
            bootstrapping_operations[operation] = bootstrapping_latency;
        }
    }
}

void ListScheduler::start_bootstrapping_ready_operations_for_unlimited_model()
{
    for (auto operation : bootstrapping_queue)
    {
        bootstrapping_operations[operation] = bootstrapping_latency;
    }
    bootstrapping_queue.clear();
}

void ListScheduler::start_bootstrapping_ready_operations_for_limited_model()
{
    auto available_core_num = get_available_core_num();
    while (available_core_num != -1 && !bootstrapping_queue.empty())
    {
        auto operation = *(bootstrapping_queue.begin());
        operation->bootstrap_start_time = clock_cycle;
        operation->core_num = available_core_num;
        bootstrapping_operations[operation] = bootstrapping_latency;
        bootstrapping_queue.erase(bootstrapping_queue.begin());
        available_core_num = get_available_core_num();
    }
}

// int ListScheduler::get_available_bootstrap_core_num()
// {
//     auto available_cores = cores;
//     for (auto &[operation, _] : bootstrapping_operations)
//     {
//         remove_element_from_vector(available_cores, operation->core_num);
//     }
//     if (available_cores.empty())
//     {
//         return -1;
//     }
//     else
//     {
//         return available_cores[0];
//     }
// }

int ListScheduler::get_available_core_num()
{
    auto available_cores = cores;
    if (create_core_assignments)
    {
        for (auto &[operation, _] : running_operations)
        {
            remove_element_from_vector(available_cores, operation->core_num);
        }
    }
    for (auto &[operation, _] : bootstrapping_operations)
    {
        remove_element_from_vector(available_cores, operation->core_num);
    }
    if (available_cores.empty())
    {
        return -1;
    }
    else
    {
        return available_cores[0];
    }
}

bool ListScheduler::core_is_available(int core_num)
{
    if (create_core_assignments)
    {
        for (auto &[operation, _] : running_operations)
        {
            if (operation->core_num == core_num)
            {
                return false;
            }
        }
    }
    for (auto &[operation, _] : bootstrapping_operations)
    {
        if (operation->core_num == core_num)
        {
            return false;
        }
    }
    return true;
}

void ListScheduler::write_lgr_like_format(std::string output_file_path)
{
    std::ofstream output_file;
    output_file.open(output_file_path);

    output_file << "Objective value: " << solver_latency << ".0" << std::endl;

    if (lgr_parser.used_selective_model)
    {
        for (auto operation : operations)
        {
            for (auto child : operation->child_ptrs_that_receive_bootstrapped_result)
            {
                output_file << "BOOTSTRAPPED( OP" << operation->id << ", OP" << child->id << ") 1" << std::endl;
            }
        }
    }
    else
    {
        for (auto operation : operations)
        {
            if (operation_is_bootstrapped(operation))
            {
                output_file << "BOOTSTRAPPED( OP" << operation->id << ") 1" << std::endl;
            }
        }
    }

    for (auto operation : operations)
    {
        output_file << "START_TIME( OP" << operation->id << ") " << operation->start_time << std::endl;
    }

    for (auto operation : operations)
    {
        if (operation->core_num > 0)
        {
            output_file << "B2C( OP" << operation->id << ", C" << operation->core_num << ") 1" << std::endl;
        }
    }

    for (auto operation : operations)
    {
        if (operation->bootstrap_start_time > 0)
        {
            output_file << "BOOTSTRAP_START_TIME( OP" << operation->id << ") " << operation->bootstrap_start_time << std::endl;
        }
    }

    output_file << "FINISH_TIME( OP0) " << solver_latency << std::endl;

    output_file.close();
}

void ListScheduler::write_assembly_like_format(std::string output_file_path)
{
    for (auto schedule : core_schedules)
    {
        std::cout << schedule;
    }
}

void ListScheduler::choose_operations_to_bootstrap()
{
    // while (!bootstrapping_paths_are_satisfied(bootstrapping_paths))
    // {
    //     update_all_ESTs_and_LSTs();
    //     update_all_ranks();
    //     choose_operation_to_bootstrap_based_on_score();
    // }
    while (!bootstrapping_paths_are_satisfied(bootstrapping_paths))
    {
        update_all_ESTs_and_LSTs();
        update_all_ranks();
        update_all_bootstrap_urgencies();
        choose_operation_to_bootstrap_based_on_score();
    }
}

void ListScheduler::update_all_bootstrap_urgencies()
{
    for (auto &operation : operations)
    {
        operation->bootstrap_urgency = 0;
    }

    for (auto &path : bootstrapping_paths)
    {
        if (!bootstrapping_path_is_satisfied(path))
        {
            for (auto operation_it = path.begin(); operation_it != path.end(); operation_it++)
            {
                auto operation = *operation_it;
                auto path_up_to_operation = OperationList(path.begin(), operation_it + 1);
                auto path_cost = get_path_cost(path_up_to_operation);
                operation->bootstrap_urgency = std::max(operation->bootstrap_urgency, path_cost);
            }
        }
    }

    for (auto &path : bootstrapping_paths)
    {
        if (!bootstrapping_path_is_satisfied(path))
        {
            for (auto operation_it = path.begin(); operation_it != path.end() - 1; operation_it++)
            {
                auto earlier_operation_urgency = (*operation_it)->bootstrap_urgency;
                auto later_operation = *(operation_it + 1);
                if (later_operation_exceeds_urgency_threshold(later_operation, earlier_operation_urgency))
                {
                    later_operation->bootstrap_urgency = -1;
                }
            }
        }
    }
}

bool ListScheduler::later_operation_exceeds_urgency_threshold(OperationPtr &later_operation, float &earlier_operation_urgency)
{
    if (earlier_operation_urgency == -1)
    {
        return true;
    }

    float urgency_to_add;
    if (later_operation->type == "MUL")
    {
        urgency_to_add = 1;
    }
    else
    {
        urgency_to_add = 1 / addition_divider;
    }

    float later_operation_urgency = earlier_operation_urgency + urgency_to_add;

    return later_operation_urgency > bootstrapping_path_threshold;
}

// void ListScheduler::update_bootstrap_urgency(OperationPtr operation)
// {

// }

void ListScheduler::choose_operation_to_bootstrap_based_on_score()
{
    auto max_score = -1;
    OperationPtr max_score_operation;
    for (auto &operation : operations)
    {
        if (!operation_is_bootstrapped(operation))
        {
            auto score = get_score(operation);
            if (score > max_score)
            {
                max_score = score;
                max_score_operation = operation;
            }
        }
    }

    max_score_operation->child_ptrs_that_receive_bootstrapped_result = max_score_operation->child_ptrs;
}

int ListScheduler::get_score(OperationPtr operation)
{
    auto num_paths = get_num_bootstrapping_paths_containing_operation(operation);

    if (num_paths == 0)
    {
        return -1;
    }

    return std::max(num_paths_multiplier * num_paths +
                        rank_multiplier * operation->rank,
                    0);

    // return num_paths_multiplier * num_paths +
    //        urgency_multiplier * operation->bootstrap_urgency;

    // return operation->bootstrap_urgency;
}

int ListScheduler::get_num_bootstrapping_paths_containing_operation(OperationPtr operation)
{
    int num_paths = 0;
    for (auto path : bootstrapping_paths)
    {
        if (lgr_parser.used_selective_model)
        {
            path.pop_back();
        }

        if (vector_contains_element(path, operation) && !bootstrapping_path_is_satisfied(path))
        {
            num_paths++;
        }
    }
    return num_paths;
}

void ListScheduler::perform_list_scheduling()
{
    if (lgr_file_path == "NULL")
    {
        choose_operations_to_bootstrap();
    }
    update_all_ESTs_and_LSTs();
    update_all_ranks();
    create_schedule();
    if (create_core_assignments)
    {
        generate_core_assignments();
    }
    else
    {
        generate_start_times_and_solver_latency();
    }
}

int main(int argc, char **argv)
{
    if (argc != 6)
    {
        std::cout << "Usage: " << argv[0] << " <dag_file> <lgr_file or \"NULL\"> <output_file> <num_cores> <create_core_assignments>" << std::endl;
        return 1;
    }

    std::string dag_file_path = argv[1];
    std::string lgr_file_path = argv[2];
    std::string output_file_path = argv[3];
    int num_cores = std::stoi(argv[4]);
    bool create_core_assignments = std::string(argv[5]) == "True";

    ListScheduler list_scheduler = ListScheduler(dag_file_path, lgr_file_path, num_cores, create_core_assignments);

    list_scheduler.perform_list_scheduling();
    list_scheduler.write_lgr_like_format(output_file_path);
    if (create_core_assignments)
    {
        list_scheduler.write_assembly_like_format(output_file_path);
    }

    return 0;
}