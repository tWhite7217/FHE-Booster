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
        for (int i = 1; i <= options.num_threads; i++)
        {
            core_availability[i] = true;
        }
    }
    else
    {
        std::cout << help_info << std::endl;
        std::cout << "num_threads must be greater than 0.";
        exit(0);
    }
}

void ListScheduler::run_simulation()
{
    initialize_simulation_state();

    while (program_is_not_finished())
    {
        update_simulation_state();
    }
    solver_latency = clock_cycle;
}

void ListScheduler::initialize_simulation_state()
{
    clock_cycle = 0;
    prioritized_unstarted_operations.clear();
    prioritized_unstarted_operations.insert(program.begin(), program.end());
    running_operations.clear();
    bootstrapping_operations.clear();

    initialize_pred_count();
    update_ready_operations();
}

void ListScheduler::initialize_pred_count()
{
    for (const auto operation : program)
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

void ListScheduler::update_simulation_state()
{
    start_ready_operations();

    clock_cycle++;

    finished_running_operations = handle_started_operations(running_operations);
    finished_bootstrapping_operations = handle_started_operations(bootstrapping_operations);

    update_pred_count();
    update_ready_operations();

    mark_cores_available(finished_bootstrapping_operations);
    mark_cores_available(finished_running_operations);

    start_bootstrapping_necessary_operations();
}

void ListScheduler::mark_cores_available(const OpSet &finished_operations)
{
    for (const auto &op : finished_operations)
    {
        core_availability[op->core_num] = true;
    }
}

bool ListScheduler::program_is_not_finished() const
{
    return !prioritized_unstarted_operations.empty() ||
           !ready_operations.empty() ||
           !running_operations.empty() ||
           !bootstrapping_operations.empty();
}

OpSet ListScheduler::handle_started_operations(std::map<OperationPtr, int> &started_operations)
{
    decrement_cycles_left(started_operations);
    auto finished_operations = get_finished_operations(started_operations);
    utl::remove_key_subset_from_map(started_operations, finished_operations);
    return finished_operations;
}

void ListScheduler::start_ready_operations()
{
    auto it = ready_operations.begin();
    int available_core = get_available_core_num();
    while (it != ready_operations.end() && (available_core != -1))
    {
        auto operation = *it;
        it = ready_operations.erase(it);
        operation->start_time = clock_cycle;
        running_operations[operation] = program.get_latency_of(operation->type);

        int best_core = get_best_core_for_operation(operation, available_core);
        operation->core_num = best_core;
        core_availability[best_core] = false;

        available_core = get_available_core_num();
    }
}

int ListScheduler::get_best_core_for_operation(const OperationPtr &operation, int fallback_core) const
{
    int best_core = fallback_core;
    for (const auto &parent : operation->parent_ptrs)
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

OpSet ListScheduler::get_finished_operations(std::map<OperationPtr, int> &started_operations)
{
    OpSet finished_operations;

    for (auto &[operation, time_left] : started_operations)
    {
        if (time_left == 0)
        {
            finished_operations.insert(operation);
        }
    }

    return finished_operations;
}

void ListScheduler::start_bootstrapping_necessary_operations()
{
    for (auto operation : finished_running_operations)
    {
        if (operation->is_bootstrapped())
        {
            bootstrapping_operations[operation] = bootstrap_latency;
            core_availability[operation->core_num] = false;
        }
    }
}

int ListScheduler::get_available_core_num() const
{
    for (const auto &[core_num, available] : core_availability)
    {
        if (available)
        {
            return core_num;
        }
    }
    return -1;
}

bool ListScheduler::core_is_available(int core_num) const
{
    return core_availability.at(core_num);
}

void ListScheduler::perform_list_scheduling()
{
    std::cout << "Generating schedule..." << std::endl;
    program.update_slack_for_every_operation();
    run_simulation();
    std::cout << "Done." << std::endl;
}

void ListScheduler::update_pred_count()
{
    for (auto &op : finished_running_operations)
    {
        for (const auto &child : op->child_ptrs)
        {
            if (!child->receives_bootstrapped_result_from(op) && prioritized_unstarted_operations.contains(child))
            {
                pred_count[child]--;
            }
        }
    }

    for (auto &op : finished_bootstrapping_operations)
    {
        for (const auto &child : op->bootstrap_children)
        {
            if (prioritized_unstarted_operations.contains(child))
            {
                pred_count[child]--;
            }
        }
    }
}

void ListScheduler::parse_args(int argc, char **argv)
{
    const int minimum_arguments = 3;

    if (argc < minimum_arguments)
    {
        std::cout << help_info << std::endl;
        exit(1);
    }

    options.dag_filename = argv[1];
    options.output_filename = argv[2];

    std::string options_string = utl::make_options_string(argc, argv, minimum_arguments);

    auto num_threads_string = utl::get_arg(options_string, "-t", "--num-threads", help_info);
    if (!num_threads_string.empty())
    {
        options.num_threads = std::stoi(num_threads_string);
    }

    auto bootstrap_arg = utl::get_arg(options_string, "-i", "--input-lgr", help_info);
    if (!bootstrap_arg.empty())
    {
        options.bootstrap_filename = bootstrap_arg;
    }

    auto latency_arg = utl::get_arg(options_string, "-l", "--latency-file", help_info);
    if (!latency_arg.empty())
    {
        options.latency_filename = latency_arg;
    }
}

void ListScheduler::print_options() const
{
    std::cout << "dag_filename: " << options.dag_filename << std::endl;
    std::cout << "latency_filename: " << options.latency_filename << std::endl;
    std::cout << "output_filename: " << options.output_filename << std::endl;
    std::cout << "bootstrap_filename: " << options.bootstrap_filename << std::endl;
    std::cout << "num_threads: " << options.num_threads << std::endl;
}

void ListScheduler::write_to_output_files() const
{
    auto file_writer = FileWriter(std::ref(program));
    file_writer.write_lgr_info_to_file(options.output_filename + ".lgr", solver_latency);
    file_writer.write_sched_file(options.output_filename + ".sched");
}

std::string ListScheduler::get_log_filename() const
{
    return options.output_filename + ".log";
}

int main(int argc, char **argv)
{
    ListScheduler list_scheduler = ListScheduler(argc, argv);

    std::ofstream log_file(list_scheduler.get_log_filename());

    std::function<void()> main_func = [&list_scheduler]()
    {
        list_scheduler.perform_list_scheduling();
        list_scheduler.write_to_output_files();
    };

    utl::perform_func_and_print_execution_time(main_func, log_file);

    return 0;
}