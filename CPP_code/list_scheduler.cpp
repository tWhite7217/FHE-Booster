#include "list_scheduler.h"

ListScheduler::ListScheduler(int argc, char **argv)
{
    parse_args(argc, argv);
    print_options();

    Program::ConstructorInput in;
    in.dag_filename = options.dag_filename;
    in.latency_filename = options.latency_filename;
    in.bootstrap_filename = options.bootstrap_filename;

    program = Program(in);

    bootstrap_latency = program.get_latency_of(OperationType::BOOT);

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
    for (auto operation : program)
    {
        pred_count[operation] = operation->get_parent_ptrs().size();
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
    prioritized_unstarted_operations.insert(program.begin(), program.end());
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
        core_availability[op->get_core_num()] = true;
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
        operation->set_start_time(clock_cycle);
        running_operations[operation] = program.get_latency_of(operation->get_type());
        if (create_core_assignments)
        {
            int best_core = get_best_core_for_operation(operation, available_core);
            operation->set_core_num(best_core);
            core_availability[best_core] = false;

            auto core_schedules_index = best_core - 1;

            std::string result_var = " c" + std::to_string(operation->get_id());

            std::string arg1;
            std::string arg2;

            auto num_var_parents = operation->get_parent_ptrs().size();
            auto num_const_parents = operation->get_constant_parent_ids().size();

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

            core_schedules[core_schedules_index] += operation->get_type().to_string() + result_var + arg1 + arg2 + thread + "\n";
            if (operation->is_bootstrapped())
            {
                core_schedules[core_schedules_index] += "BOOT c0" + std::to_string(operation->get_id()) + " c" + std::to_string(operation->get_id()) + " t" + std::to_string(best_core) + "\n";
            }
        }
    }
}

std::string ListScheduler::get_constant_arg(OperationPtr operation, size_t parent_num)
{
    return " k" + std::to_string(operation->get_constant_parent_ids()[parent_num]);
}

std::string ListScheduler::get_variable_arg(OperationPtr operation, size_t parent_num)
{
    std::string arg = " c";
    auto parent = operation->get_parent_ptrs()[parent_num];
    if (operation->receives_bootstrapped_result_from(parent))
    {
        arg += "0";
    }
    return arg + std::to_string(parent->get_id());
}

int ListScheduler::get_best_core_for_operation(OperationPtr operation, int fallback_core)
{
    int best_core = fallback_core;
    for (const auto &parent : operation->get_parent_ptrs())
    {
        if (core_is_available(parent->get_core_num()))
        {
            best_core = parent->get_core_num();
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
        if (operation->is_bootstrapped())
        {
            bootstrapping_queue.insert(operation);
        }
    }
}

void ListScheduler::start_bootstrapping_necessary_operations()
{
    for (auto operation : finished_running_operations)
    {
        if (operation->is_bootstrapped())
        {
            bootstrapping_operations[operation] = bootstrap_latency;
            core_availability[operation->get_core_num()] = false;
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
        operation->set_bootstrap_start_time(clock_cycle);
        operation->set_core_num(available_core_num);
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

void ListScheduler::write_sched_to_file(const std::string &filename)
{
    std::ofstream output_file(filename);

    for (auto schedule : core_schedules)
    {
        output_file << schedule;
    }
}

void ListScheduler::perform_list_scheduling()
{
    program.update_ESTs_and_LSTs();
    // program.update_slacks_and_get_maximum();
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
        for (const auto &child : op->get_child_ptrs())
        {
            if (!child->receives_bootstrapped_result_from(op) && multiset_contains_element(prioritized_unstarted_operations, child))
            {
                pred_count[child]--;
            }
        }
    }

    for (auto &op : finished_bootstrapping_operations)
    {
        for (const auto &child : op->get_bootstrap_children())
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
    options.latency_filename = argv[2];
    options.output_filename = argv[3];

    std::string options_string;
    for (auto i = 4; i < argc; i++)
    {
        options_string += std::string(argv[i]) + " ";
    }

    auto num_threads_string = get_arg(options_string, "-t", "--num-threads", help_info);
    if (!num_threads_string.empty())
    {
        options.num_threads = std::stoi(num_threads_string);
    }

    auto bootstrap_arg = get_arg(options_string, "-i", "--input-lgr", help_info);
    if (!bootstrap_arg.empty())
    {
        options.bootstrap_filename = bootstrap_arg;
    }
}

void ListScheduler::print_options()
{
    std::cout << "dag_filename: " << options.dag_filename << std::endl;
    std::cout << "latency_filename: " << options.latency_filename << std::endl;
    std::cout << "output_filename: " << options.output_filename << std::endl;
    std::cout << "bootstrap_filename: " << options.bootstrap_filename << std::endl;
    std::cout << "num_threads: " << options.num_threads << std::endl;
}

void ListScheduler::write_to_output_files()
{
    program.write_lgr_info_to_file(options.output_filename + ".lgr", solver_latency);
    write_sched_to_file(options.output_filename + ".sched");
}

int main(int argc, char **argv)
{
    ListScheduler list_scheduler = ListScheduler(argc, argv);

    list_scheduler.perform_list_scheduling();
    list_scheduler.write_to_output_files();

    return 0;
}