#include "list_scheduler.h"

ListScheduler::ListScheduler(std::string dag_file_path, std::string lgr_file_path, int num_cores, int heuristic_type)
    : lgr_file_path{lgr_file_path}, num_cores{num_cores}
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
    else
    {
        BootstrappingPathGenerator path_generator(operations, lgr_parser.used_selective_model);
        bootstrapping_paths = path_generator.get_bootstrapping_paths(dag_file_path);
        // bootstrapping_paths = path_generator.generate_bootstrapping_paths();

        switch (heuristic_type)
        {
        case 0: // num_paths with slack
            num_paths_multiplier = 12;
            rank_multiplier = 2;
            urgency_multiplier = 0;
            break;
        case 1: // urgency
            num_paths_multiplier = 0;
            rank_multiplier = 0;
            urgency_multiplier = 1;
            break;
        case 2: // urgency and num_paths
            num_paths_multiplier = 1;
            rank_multiplier = 0;
            urgency_multiplier = 5;
            break;
        case 3: // num_paths minus slack
            num_paths_multiplier = 25;
            rank_multiplier = -1;
            urgency_multiplier = 0;
            break;
        case 4: // num_paths minus urgency
            num_paths_multiplier = 25;
            rank_multiplier = 0;
            urgency_multiplier = -1;
            break;
        }
    }

    if (num_cores > 0)
    {
        core_schedules.assign(num_cores, "");
        for (auto i = 1; i <= num_cores; i++)
        {
            core_availability[i] = true;
        }
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

void ListScheduler::update_all_ranks()
{
    for (auto &operation : operations)
    {
        operation->rank = operation->latest_start_time - operation->earliest_start_time;
    }
}

void ListScheduler::initialize_pred_count()
{
    for (auto operation : operations)
    {
        pred_count[operation] = operation->parent_ptrs.size();
    }
}

void ListScheduler::update_ready_operations()
{
    auto it = prioritized_unstarted_operations.begin();
    while (it != prioritized_unstarted_operations.end())
    {
        auto operation = *it;
        if (pred_count[operation] == 0)
        {
            ready_operations.push_back(operation);
            it = prioritized_unstarted_operations.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void ListScheduler::generate_start_times_and_solver_latency()
{
    create_core_assignments = false;
    clock_cycle = 0;
    running_operations.clear();
    bootstrapping_operations.clear();
    bootstrapping_queue.clear();
    prioritized_unstarted_operations.insert(operations.begin(), operations.end());

    initialize_pred_count();
    update_ready_operations();

    while (program_is_not_finished())
    {
        start_ready_operations();

        clock_cycle++;

        auto finished_bootstrapping_operations = handle_started_operations(bootstrapping_operations);

        if (num_cores > 0)
        {
            mark_cores_available(finished_bootstrapping_operations);
        }

        auto finished_running_operations = handle_started_operations(running_operations);
        add_necessary_operations_to_bootstrapping_queue(finished_running_operations);
        start_bootstrapping_ready_operations();

        update_pred_count(finished_running_operations, finished_bootstrapping_operations);
        update_ready_operations();
    }
    solver_latency = clock_cycle;
}

void ListScheduler::generate_core_assignments()
{
    create_core_assignments = true;
    clock_cycle = 0;
    running_operations.clear();
    bootstrapping_operations.clear();
    prioritized_unstarted_operations.insert(operations.begin(), operations.end());

    initialize_pred_count();
    update_ready_operations();

    while (program_is_not_finished())
    {
        start_ready_operations();

        clock_cycle++;

        auto finished_bootstrapping_operations = handle_started_operations(bootstrapping_operations);
        auto finished_running_operations = handle_started_operations(running_operations);

        if (num_cores > 0)
        {
            mark_cores_available(finished_bootstrapping_operations);
            mark_cores_available(finished_running_operations);
        }

        start_bootstrapping_necessary_operations(finished_running_operations);

        update_pred_count(finished_running_operations, finished_bootstrapping_operations);
        update_ready_operations();
    }
}

void ListScheduler::mark_cores_available(std::unordered_set<OperationPtr> &finished_operations)
{
    for (auto &op : finished_operations)
    {
        core_availability[op->core_num] = true;
    }
}

bool ListScheduler::program_is_not_finished()
{
    return !prioritized_unstarted_operations.empty() ||
           !ready_operations.empty() ||
           !running_operations.empty() ||
           !bootstrapping_queue.empty() ||
           !bootstrapping_operations.empty();
}

std::unordered_set<OperationPtr> ListScheduler::handle_started_operations(std::map<OperationPtr, int> &started_operations)
{
    decrement_cycles_left(started_operations);
    auto finished_operations = get_finished_operations(started_operations);
    remove_key_subset_from_map(started_operations, finished_operations);
    return finished_operations;
}

void ListScheduler::start_ready_operations()
{
    auto it = ready_operations.begin();
    int available_core;
    while (it != ready_operations.end() && (!create_core_assignments || (available_core = get_available_core_num()) != -1))
    {
        auto operation = *it;
        it = ready_operations.erase(it);
        operation->start_time = clock_cycle;
        running_operations[operation] = operation_type_to_latency_map[operation->type];
        if (create_core_assignments)
        {
            int best_core = get_best_core_for_operation(operation, available_core);
            operation->core_num = best_core;
            core_availability[best_core] = false;

            auto core_schedules_index = best_core - 1;

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

            core_schedules[core_schedules_index] += operation->type + result_var + arg1 + arg2 + thread + "\n";
            if (operation_is_bootstrapped(operation))
            {
                core_schedules[core_schedules_index] += "BOOT c0" + std::to_string(operation->id) + " c" + std::to_string(operation->id) + " t" + std::to_string(best_core) + "\n";
            }
        }
    }
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

std::unordered_set<OperationPtr> ListScheduler::get_finished_operations(std::map<OperationPtr, int> &started_operations)
{
    std::unordered_set<OperationPtr> finished_operations;

    for (auto &[operation, time_left] : started_operations)
    {
        if (time_left == 0)
        {
            finished_operations.insert(operation);
        }
    }

    return finished_operations;
}

void ListScheduler::add_necessary_operations_to_bootstrapping_queue(std::unordered_set<OperationPtr> finished_running_operations)
{
    for (auto operation : finished_running_operations)
    {
        if (operation_is_bootstrapped(operation))
        {
            bootstrapping_queue.insert(operation);
        }
    }
}

void ListScheduler::start_bootstrapping_necessary_operations(std::unordered_set<OperationPtr> finished_running_operations)
{
    for (auto operation : finished_running_operations)
    {
        if (operation_is_bootstrapped(operation))
        {
            bootstrapping_operations[operation] = bootstrapping_latency;
            core_availability[operation->core_num] = false;
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
        core_availability[available_core_num] = false;
        bootstrapping_operations[operation] = bootstrapping_latency;
        bootstrapping_queue.erase(bootstrapping_queue.begin());
        available_core_num = get_available_core_num();
    }
}

int ListScheduler::get_available_core_num()
{
    for (auto &[core_num, available] : core_availability)
    {
        if (available)
        {
            return core_num;
        }
    }
    return -1;
}

bool ListScheduler::core_is_available(int core_num)
{
    return core_availability[core_num];
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
    std::ofstream output_file;
    output_file.open(output_file_path);

    for (auto schedule : core_schedules)
    {
        output_file << schedule;
        // std::cout << schedule;
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
    auto count = 0;
    while (!bootstrapping_paths_are_satisfied(bootstrapping_paths))
    {
        // std::cout << count << std::endl;
        // count++;
        // if (num_paths_multiplier != 0)
        // {
        update_num_paths_for_every_operation();
        // }
        if (rank_multiplier != 0)
        {
            update_all_ESTs_and_LSTs();
            update_all_ranks();
        }
        if (urgency_multiplier != 0)
        {
            update_all_bootstrap_urgencies();
        }
        choose_operation_to_bootstrap_based_on_score();
    }
}

void ListScheduler::update_all_bootstrap_urgencies()
{
    for (auto &operation : operations)
    {
        operation->bootstrap_urgency = -1;
    }

    auto count = 0;
    for (auto &path : bootstrapping_paths)
    {
        if (path_is_urgent(path))
        {
            count++;
            path.back()->bootstrap_urgency = 1;
        }
    }
    // std::cout << count << std::endl;
    if (count == 0)
    {
        std::cout << "here" << std::endl;
    }
}

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

    // std::cout << max_score << std::endl;
    // std::cout << max_score_operation->child_ptrs.size() << std::endl;
    max_score_operation->child_ptrs_that_receive_bootstrapped_result = max_score_operation->child_ptrs;
}

int ListScheduler::get_score(OperationPtr operation)
{
    if (operation->num_unsatisfied_paths == 0)
    {
        return -1;
    }

    return std::max(num_paths_multiplier * operation->num_unsatisfied_paths +
                        rank_multiplier * operation->rank +
                        urgency_multiplier * operation->bootstrap_urgency,
                    0.0);
}

void ListScheduler::update_num_paths_for_every_operation()
{
    for (auto &operation : operations)
    {
        operation->num_unsatisfied_paths = 0;
    }

    for (auto path : bootstrapping_paths)
    {
        if (lgr_parser.used_selective_model)
        {
            path.pop_back();
        }

        if (!bootstrapping_path_is_satisfied(path))
        {
            for (auto &operation : path)
            {
                operation->num_unsatisfied_paths++;
            }
        }
    }
}

void ListScheduler::perform_list_scheduling()
{
    std::cout << "here1" << std::endl;
    if (lgr_file_path == "NULL")
    {
        choose_operations_to_bootstrap();
    }
    std::cout << "here2" << std::endl;
    update_all_ESTs_and_LSTs();
    std::cout << "here3" << std::endl;
    update_all_ranks();
    std::cout << "here4" << std::endl;
    generate_start_times_and_solver_latency();
    std::cout << "here5" << std::endl;
    if (num_cores > 0)
    {
        generate_core_assignments();
    }
    std::cout << "here6" << std::endl;
}

void ListScheduler::update_pred_count(std::unordered_set<OperationPtr> &finished_running_operations, std::unordered_set<OperationPtr> &finished_bootstrapping_operations)
{
    std::unordered_set<OperationPtr> readied_operations;
    for (auto &op : finished_running_operations)
    {
        for (auto &child : op->child_ptrs)
        {
            if (!operation_receives_a_bootstrapped_result_from_parent(child, op) && multiset_contains_element(prioritized_unstarted_operations, child))
            {
                pred_count[child]--;
            }
        }
    }

    for (auto &op : finished_bootstrapping_operations)
    {
        for (auto &child : op->child_ptrs)
        {
            if (multiset_contains_element(prioritized_unstarted_operations, child))
            {
                pred_count[child]--;
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 6)
    {
        std::cout << "Usage: " << argv[0] << " <dag_file> <lgr_file or \"NULL\"> <output_file> <num_cores> <heuristic_type>" << std::endl;
        return 1;
    }

    std::string dag_file_path = argv[1];
    std::string lgr_file_path = argv[2];
    std::string output_file_path = argv[3];
    int num_cores = std::stoi(argv[4]);
    int heuristic_type = std::stoi(argv[5]);

    if (lgr_file_path == "NULL" && heuristic_type < 0 || heuristic_type > 4)
    {
        std::cout << "Invalid heuristic type." << std::endl;
        return 1;
    }

    ListScheduler list_scheduler = ListScheduler(dag_file_path, lgr_file_path, num_cores, heuristic_type);

    list_scheduler.perform_list_scheduling();
    list_scheduler.write_lgr_like_format(output_file_path + ".lgr");
    list_scheduler.write_assembly_like_format(output_file_path + ".sched");

    return 0;
}