#include "list_scheduling.h"

ListScheduler::ListScheduler(std::string dag_file_path, std::string lgr_file_path, int num_cores)
    : num_cores{num_cores}
{
    InputParser input_parser;
    input_parser.parse_input_to_generate_operations(dag_file_path);
    operations = input_parser.get_operations();
    operation_type_to_latency_map = input_parser.get_operation_type_to_latency_map();

    lgr_parser.switchIstream(lgr_file_path);
    lgr_parser.set_operations(operations);
    lgr_parser.lex();
    used_selective_model = lgr_parser.used_selective_model;

    BootstrappingPathGenerator path_generator(operations, used_selective_model);
    bootstrapping_paths = path_generator.generate_bootstrapping_paths();

    if (num_cores > 0)
    {
        cores.resize(num_cores);
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

void ListScheduler::update_earliest_start_time(OperationPtr operation)
{
    for (auto parent : operation->parent_ptrs)
    {
        int parent_latency = operation_type_to_latency_map[parent->type];
        if (lgr_parser.operation_is_bootstrapped(parent, operation))
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

void ListScheduler::generate_child_ptrs()
{
    for (auto operation : operations)
    {
        for (auto potential_child_operation : operations)
        {
            if (vector_contains_element(potential_child_operation->parent_ptrs, operation))
            {
                operation->child_ptrs.push_back(potential_child_operation);
            }
        }
    }
}

void ListScheduler::update_latest_start_time(OperationPtr operation, int earliest_possible_program_end_time)
{
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
            if (lgr_parser.operation_is_bootstrapped(child))
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
    auto operations_in_dag_order = get_operations_in_dag_order();

    for (auto operation : operations_in_dag_order)
    {
        update_earliest_start_time(operation);
    }

    int earliest_program_end_time = get_earliest_possible_program_end_time();

    auto operations_in_reverse_dag_order = operations_in_dag_order;
    std::reverse(operations_in_reverse_dag_order.begin(), operations_in_reverse_dag_order.end());

    for (auto operation : operations_in_reverse_dag_order)
    {
        update_latest_start_time(operation, earliest_program_end_time);
    }
}

std::vector<OperationPtr> ListScheduler::get_operations_in_dag_order()
{
    std::vector<OperationPtr> operations_in_dag_order;
    for (auto operation : operations)
    {
        if (operation->parent_ptrs.empty())
        {
            operations_in_dag_order.push_back(operation);
        }
    }
    return operations_in_dag_order;
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
            (lgr_parser.operation_is_bootstrapped(parent, operation) &&
             map_contains_key(bootstrapping_operations, parent)))
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

    while (!ordered_unstarted_operations.empty())
    {
        start_ready_operations();

        clock_cycle++;

        handle_started_operations(bootstrapping_operations);

        auto finished_running_ids = handle_started_operations(running_operations);
        add_necessary_operations_to_bootstrapping_queue(finished_running_ids);
        start_bootstrapping_ready_operations();
    }
    clock_cycle += map_max_value(running_operations);
    solver_latency = clock_cycle;
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
        if (operation_is_ready(operation))
        {
            operation->start_time = clock_cycle;
            running_operations[operation] = operation_type_to_latency_map[operation->type];
            started_operation_ids.insert(operation);
        }
    }

    remove_element_subset_from_vector(ordered_unstarted_operations, started_operation_ids);
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
        if (lgr_parser.operation_is_bootstrapped(operation))
        {
            bootstrapping_queue.insert(operation);
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
    auto available_core_num = get_available_bootstrap_core_num();
    while (available_core_num != -1)
    {
        auto operation = *bootstrapping_queue.begin();
        operation->bootstrap_start_time = clock_cycle;
        operation->core_num = available_core_num;
        bootstrapping_operations[operation] = bootstrapping_latency;
        bootstrapping_queue.erase(bootstrapping_queue.begin());
        available_core_num = get_available_bootstrap_core_num();
    }
}

int ListScheduler::get_available_bootstrap_core_num()
{
    auto available_cores = cores;
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

void ListScheduler::write_lgr_like_format(std::string output_file_path)
{
    std::ofstream output_file;
    output_file.open(output_file_path, std::ios::out);

    for (auto operation : lgr_parser.bootstrapped_operations)
    {
        output_file << "BOOTSTRAPPED( OP" << operation->id << ") 1" << std::endl;
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

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        std::cout << "Usage: " << argv[0] << " <dag_file> <lgr_file> <output_file> <num_cores>" << std::endl;
        return 1;
    }

    std::string dag_file_path = argv[1];
    std::string lgr_file_path = argv[2];
    std::string output_file_path = argv[3];
    int num_cores = std::stoi(argv[4]);

    ListScheduler list_scheduler = ListScheduler(dag_file_path, lgr_file_path, num_cores);

    list_scheduler.generate_child_ptrs();
    list_scheduler.update_all_ESTs_and_LSTs();
    list_scheduler.update_all_ranks();
    list_scheduler.create_schedule();
    list_scheduler.generate_start_times_and_solver_latency();
    list_scheduler.write_lgr_like_format(output_file_path);

    return 0;
}
