#include "list_scheduler.h"

ListScheduler::ListScheduler(int argc, char **argv)
{
    parse_args(argc, argv);
    print_options();

    InputParser input_parser;
    input_parser.parse_input_to_generate_operations(options.dag_filename);
    operations = input_parser.get_operations();
    operation_type_to_latency_map = input_parser.get_operation_type_to_latency_map();

    if (options.lgr_filename != "NULL")
    {
        lgr_parser.switchIstream(options.lgr_filename);
        lgr_parser.set_operations(operations);
        lgr_parser.lex();
    }

    const auto num_threads = options.num_threads;
    if (num_threads > 0)
    {
        core_schedules.assign(num_threads, "");
        for (auto i = 1; i <= options.num_threads; i++)
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
    initialize_simulation_state();

    while (program_is_not_finished())
    {
        update_simulation_state();

        if (options.num_threads > 0)
        {
            mark_cores_available(finished_bootstrapping_operations);
        }

        add_necessary_operations_to_bootstrapping_queue();
        start_bootstrapping_ready_operations();
    }
    solver_latency = clock_cycle;
}

void ListScheduler::generate_core_assignments()
{
    create_core_assignments = true;
    initialize_simulation_state();

    while (program_is_not_finished())
    {
        update_simulation_state();

        mark_cores_available(finished_bootstrapping_operations);
        mark_cores_available(finished_running_operations);

        start_bootstrapping_necessary_operations();
    }
}

void ListScheduler::initialize_simulation_state()
{
    clock_cycle = 0;
    prioritized_unstarted_operations.clear();
    prioritized_unstarted_operations.insert(operations.begin(), operations.end());
    running_operations.clear();
    bootstrapping_queue.clear();
    bootstrapping_operations.clear();

    initialize_pred_count();
    update_ready_operations();
}

void ListScheduler::update_simulation_state()
{
    start_ready_operations();

    clock_cycle++;

    finished_running_operations = handle_started_operations(running_operations);
    finished_bootstrapping_operations = handle_started_operations(bootstrapping_operations);

    update_pred_count();
    update_ready_operations();
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

            std::string result_var = " c" + std::to_string(operation->id);

            std::string arg1;
            std::string arg2;

            auto num_var_parents = operation->parent_ptrs.size();
            auto num_const_parents = operation->constant_parent_ids.size();

            if (num_var_parents == 2)
            {
                arg1 = get_variable_arg(operation, 0);
                arg2 = get_variable_arg(operation, 1);
            }
            else if (num_const_parents == 2)
            {
                arg1 = get_constant_arg(operation, 0);
                arg2 = get_constant_arg(operation, 1);
            }
            else if (num_var_parents == 1 && num_const_parents == 1)
            {
                arg1 = get_variable_arg(operation, 0);
                arg2 = get_constant_arg(operation, 0);
            }
            else if (num_var_parents == 1)
            {
                arg1 = arg2 = get_variable_arg(operation, 0);
            }
            else if (num_const_parents == 1)
            {
                arg1 = arg2 = get_constant_arg(operation, 0);
            }
            else
            {
                throw std::runtime_error("An operation must have at least one input.");
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

std::string ListScheduler::get_constant_arg(OperationPtr operation, size_t parent_num)
{
    return " k" + std::to_string(operation->constant_parent_ids[parent_num]);
}

std::string ListScheduler::get_variable_arg(OperationPtr operation, size_t parent_num)
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

void ListScheduler::add_necessary_operations_to_bootstrapping_queue()
{
    for (auto operation : finished_running_operations)
    {
        if (operation_is_bootstrapped(operation))
        {
            bootstrapping_queue.insert(operation);
        }
    }
}

void ListScheduler::start_bootstrapping_necessary_operations()
{
    for (auto operation : finished_running_operations)
    {
        if (operation_is_bootstrapped(operation))
        {
            bootstrapping_operations[operation] = bootstrap_latency;
            core_availability[operation->core_num] = false;
        }
    }
}

void ListScheduler::start_bootstrapping_ready_operations_for_unlimited_model()
{
    for (auto operation : bootstrapping_queue)
    {
        bootstrapping_operations[operation] = bootstrap_latency;
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
        bootstrapping_operations[operation] = bootstrap_latency;
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

void ListScheduler::write_lgr_like_format()
{
    std::ofstream output_file;
    output_file.open(options.output_filename + ".lgr");

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

void ListScheduler::write_assembly_like_format()
{
    std::ofstream output_file;
    output_file.open(options.output_filename + ".sched");

    for (auto schedule : core_schedules)
    {
        output_file << schedule;
    }
}

void ListScheduler::perform_list_scheduling()
{
    update_all_ESTs_and_LSTs(operations, operation_type_to_latency_map);
    update_all_slacks(operations);
    std::cout << "Generating lgr..." << std::endl;
    generate_start_times_and_solver_latency();
    std::cout << "Done." << std::endl;
    std::cout << "Generating sched..." << std::endl;
    if (options.num_threads > 0)
    {
        generate_core_assignments();
    }
    std::cout << "Done." << std::endl;
}

void ListScheduler::update_pred_count()
{
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
        for (auto &child : op->child_ptrs_that_receive_bootstrapped_result)
        {
            if (multiset_contains_element(prioritized_unstarted_operations, child))
            {
                pred_count[child]--;
            }
        }
    }
}

void ListScheduler::parse_args(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << help_info << std::endl;
        exit(1);
    }

    options.dag_filename = argv[1];
    options.output_filename = argv[2];

    std::string options_string;
    for (auto i = 3; i < argc; i++)
    {
        options_string += std::string(argv[i]) + " ";
    }

    auto num_threads_string = get_arg(options_string, "-t", "--num-threads", help_info);
    if (!num_threads_string.empty())
    {
        options.num_threads = std::stoi(num_threads_string);
    }

    auto lgr_arg = get_arg(options_string, "-i", "--input-lgr", help_info);
    if (!lgr_arg.empty())
    {
        options.lgr_filename = lgr_arg;
    }
}

void ListScheduler::print_options()
{
    std::cout << "dag_filename: " << options.dag_filename << std::endl;
    std::cout << "output_filename: " << options.output_filename << std::endl;
    std::cout << "lgr_filename: " << options.lgr_filename << std::endl;
    std::cout << "num_threads: " << options.num_threads << std::endl;
}

int main(int argc, char **argv)
{
    ListScheduler list_scheduler = ListScheduler(argc, argv);

    list_scheduler.perform_list_scheduling();
    list_scheduler.write_lgr_like_format();
    list_scheduler.write_assembly_like_format();

    return 0;
}