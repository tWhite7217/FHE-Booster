#include "list_scheduling.h"

ListScheduler::ListScheduler(std::string dag_file_path, std::string lgr_file_path, int num_cores)
    : num_cores{num_cores}
{
    lgr_parser.switchIstream(lgr_file_path);
    lgr_parser.lex();
    used_selective_model = lgr_parser.used_selective_model;

    InputParser input_parser = InputParser(false, used_selective_model);
    input_parser.parse_input(dag_file_path);

    operations = input_parser.get_operations();
    operation_type_to_latency_map = input_parser.get_operation_type_to_latency_map();
    bootstrapping_paths = input_parser.get_bootstrapping_paths();

    if (num_cores > 0)
    {
        cores.resize(num_cores);
        std::iota(cores.begin(), cores.end(), 1);
        start_bootstrapping_ready_operations = [this]()
        { start_bootstrapping_ready_operations_for_limited_model; };
    }
    else
    {
        start_bootstrapping_ready_operations = [this]()
        { start_bootstrapping_ready_operations_for_unlimited_model; };
    }
}

void ListScheduler::update_earliest_start_time(Operation &operation)
{
    for (auto parent_id : operation.parent_ids)
    {
        auto parent_operation = operations.get(parent_id);
        int parent_latency = operation_type_to_latency_map[parent_operation.type];
        if (lgr_parser.operation_is_bootstrapped(parent_id, operation.id))
        {
            parent_latency += bootstrapping_latency;
        }
        int possible_est =
            parent_operation.earliest_start_time +
            parent_latency;
        if (possible_est > operation.earliest_start_time)
        {
            operation.earliest_start_time = possible_est;
        }
    }
}

int ListScheduler::get_earliest_possible_program_end_time()
{
    int earliest_possible_program_end_time = 0;
    for (auto operation : operations.get_iterable_list())
    {
        auto latency = operation_type_to_latency_map[operation.type];
        int operation_earliest_end_time = operation.earliest_start_time + latency;
        if (operation_earliest_end_time > earliest_possible_program_end_time)
        {
            earliest_possible_program_end_time = operation_earliest_end_time;
        }
    }
    return earliest_possible_program_end_time;
}

void ListScheduler::generate_child_ids()
{
    for (auto &operation : operations.get_iterable_list())
    {
        for (auto potential_child_operation : operations.get_iterable_list())
        {
            if (vector_contains_element(potential_child_operation.parent_ids, operation.id))
            {
                operation.child_ids.push_back(potential_child_operation.id);
            }
        }
    }
}

void ListScheduler::update_latest_start_time(Operation &operation, int earliest_possible_program_end_time)
{
    if (operation.child_ids.empty())
    {
        operation.latest_start_time =
            earliest_possible_program_end_time - operation_type_to_latency_map[operation.type];
    }
    else
    {
        for (auto child_id : operation.child_ids)
        {
            Operation child_operation = operations.get(child_id);
            int child_latency = operation_type_to_latency_map[child_operation.type];
            if (lgr_parser.operation_is_bootstrapped(child_id))
            {
                child_latency += bootstrapping_latency;
            }
            int possible_lst =
                child_operation.latest_start_time -
                child_latency;
            if (possible_lst < operation.latest_start_time)
            {
                operation.latest_start_time = possible_lst;
            }
        }
    }
}

void ListScheduler::update_all_ESTs_and_LSTs()
{
    auto num_operations = operations.size();

    auto forward_range = std::vector{num_operations};
    std::iota(forward_range.begin(), forward_range.end(), 1);

    for (auto id : forward_range)
    {
        update_earliest_start_time(operations.get(id));
    }

    int earliest_end_time = get_earliest_possible_program_end_time();

    auto backward_range = forward_range;
    std::reverse(backward_range.begin(), backward_range.end());

    for (auto id : backward_range)
    {
        update_latest_start_time(operations.get(id), earliest_end_time);
    }
}

void ListScheduler::update_all_ranks()
{
    for (auto &operation : operations.get_iterable_list())
    {
        operation.rank = operation.latest_start_time - operation.earliest_start_time;
    }
}

std::vector<int> ListScheduler::get_priority_list()
{
    std::vector<int> priority_list = std::vector<int>(operations.size());
    std::iota(priority_list.begin(), priority_list.end(), 1);

    std::sort(priority_list.begin(), priority_list.end(),
              [this](int a, int b)
              {
                  auto operation_a = operations.get(a);
                  auto operation_b = operations.get(b);
                  return operation_a.rank > operation_b.rank;
              });
    return priority_list;
}

std::map<int, int> ListScheduler::initialize_pred_count()
{
    std::map<int, int> pred_count;
    for (auto operation : operations.get_iterable_list())
    {
        pred_count[operation.id] = operation.parent_ids.size();
    }
    return pred_count;
}

std::set<int> ListScheduler::initialize_ready_operations(std::map<int, int> pred_count)
{
    std::set<int> ready_operations;
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
    std::vector<int> priority_list = get_priority_list();
    std::map<int, int> pred_count = initialize_pred_count();
    std::set<int> ready_operations = initialize_ready_operations(pred_count);

    while (!ready_operations.empty())
    {
        for (const auto id : priority_list)
        {
            if (set_contains_element(ready_operations, id))
            {
                schedule.push_back(id);
                ready_operations.erase(id);

                auto operation = operations.get(id);
                for (auto child_id : operation.child_ids)
                {
                    pred_count[child_id]--;
                    if (pred_count[child_id] == 0)
                    {
                        ready_operations.insert(child_id);
                    }
                }
            }
        }
    }
}

bool ListScheduler::operation_is_ready(int next_operation_id, std::map<int, int> running_operations, std::vector<int> ordered_unstarted_operations, std::map<int, int> bootstrapping_operations)
{
    auto operation = operations.get(next_operation_id);
    for (auto parent_id : operation.parent_ids)
    {
        if (map_contains_key(running_operations, parent_id) ||
            vector_contains_element(ordered_unstarted_operations, parent_id) ||
            (lgr_parser.operation_is_bootstrapped(parent_id, next_operation_id) &&
             map_contains_key(bootstrapping_operations, parent_id)))
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

std::set<int> ListScheduler::handle_started_operations(std::map<int, int> &started_operations)
{
    decrement_cycles_left(started_operations);
    auto finished_ids = get_finished_operations(started_operations);
    remove_key_subset_from_map(started_operations, finished_ids);
    return finished_ids;
}

void ListScheduler::start_ready_operations()
{
    std::set<int> started_operation_ids;

    for (auto id : ordered_unstarted_operations)
    {
        if (operation_is_ready(id, running_operations, ordered_unstarted_operations, bootstrapping_operations))
        {
            Operation &next_operation = operations.get(id);
            next_operation.start_time = clock_cycle;
            running_operations[id] = operation_type_to_latency_map[next_operation.type];
            started_operation_ids.insert(id);
        }
    }

    remove_element_subset_from_vector(ordered_unstarted_operations, started_operation_ids);
}

void ListScheduler::decrement_cycles_left(std::map<int, int> &started_operations)
{
    for (auto &[id, time_left] : started_operations)
    {
        time_left--;
    }
}

std::set<int> ListScheduler::get_finished_operations(std::map<int, int> &started_operations)
{
    std::set<int> finished_ids;

    for (auto &[id, time_left] : started_operations)
    {
        if (time_left == 0)
        {
            finished_ids.insert(id);
        }
    }

    return finished_ids;
}

void ListScheduler::add_necessary_operations_to_bootstrapping_queue(std::set<int> finished_running_ids)
{
    for (auto id : finished_running_ids)
    {
        if (lgr_parser.operation_is_bootstrapped(id))
        {
            bootstrapping_queue.insert(id);
        }
    }
}

void ListScheduler::start_bootstrapping_ready_operations_for_unlimited_model()
{
    for (auto id : bootstrapping_queue)
    {
        Operation &operation = operations.get(id);
        bootstrapping_operations[id] = bootstrapping_latency;
    }
}

void ListScheduler::start_bootstrapping_ready_operations_for_limited_model()
{
    for (auto id : bootstrapping_queue)
    {
        auto bootstrap_core_num = get_available_bootstrap_core_num();
        if (bootstrap_core_num = -1)
        {
            return;
        }
        else
        {
            Operation &operation = operations.get(id);
            operation.bootstrap_start_time = clock_cycle;
            operation.core_num = bootstrap_core_num;
            bootstrapping_operations[id] = bootstrapping_latency;
        }
    }
}

int ListScheduler::get_available_bootstrap_core_num()
{
    auto available_cores = cores;
    for (auto &[id, _] : bootstrapping_operations)
    {
        remove_element_from_vector(available_cores, operations.get(id).core_num);
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

    for (auto id : lgr_parser.bootstrapped_operation_ids)
    {
        output_file << "BOOTSTRAPPED( OP" << id << ") 1" << std::endl;
    }

    for (auto operation : operations.get_iterable_list())
    {
        output_file << "START_TIME( OP" << operation.id << ") " << operation.start_time << std::endl;
    }

    for (auto operation : operations.get_iterable_list())
    {
        if (operation.core_num > 0)
        {
            output_file << "B2C( OP" << operation.id << ", C" << operation.core_num << ") 1" << std::endl;
        }
    }

    for (auto operation : operations.get_iterable_list())
    {
        if (operation.bootstrap_start_time > 0)
        {
            output_file << "BOOTSTRAP_START_TIME( OP" << operation.id << ") " << operation.bootstrap_start_time << std::endl;
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

    list_scheduler.generate_child_ids();
    list_scheduler.update_all_ESTs_and_LSTs();
    list_scheduler.update_all_ranks();
    list_scheduler.create_schedule();
    list_scheduler.generate_start_times_and_solver_latency();
    list_scheduler.write_lgr_like_format(output_file_path);

    return 0;
}
