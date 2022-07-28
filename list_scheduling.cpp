#include "list_scheduling.h"

ListScheduler::ListScheduler(std::string dag_file_path, std::string lgr_file_path)
{
    lgr_parser.switchIstream(lgr_file_path);
    lgr_parser.lex();
    used_selective_model = lgr_parser.used_selective_model;

    InputParser input_parser = InputParser(false, used_selective_model);
    input_parser.parse_input(dag_file_path);

    operations = input_parser.get_operations();
    operation_type_to_latency_map = input_parser.get_operation_type_to_latency_map();
    bootstrapping_paths = input_parser.get_bootstrapping_paths();
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

void ListScheduler::generate_start_times_and_solver_latency(int num_cores)
{
    int clock_cycle = 0;
    std::map<int, int> running_operations;
    std::map<int, int> bootstrapping_operations;
    std::vector<int> ordered_unstarted_operations = schedule;

    while (!ordered_unstarted_operations.empty())
    {
        // std::cout << "ordered_unstarted_operations size: " << ordered_unstarted_operations.size() << std::endl;
        // std::cout << "Clock cycle: " << clock_cycle << std::endl;

        std::set<int> unstarted_ids_to_remove;

        for (auto id : ordered_unstarted_operations)
        {
            if (operation_is_ready(id, running_operations, ordered_unstarted_operations, bootstrapping_operations))
            {
                Operation &next_operation = operations.get(id);
                next_operation.start_time = clock_cycle;
                running_operations[id] = operation_type_to_latency_map[next_operation.type];
                unstarted_ids_to_remove.insert(id);
            }
        }

        for (auto id : unstarted_ids_to_remove)
        {
            remove_element_from_vector(ordered_unstarted_operations, id);
        }

        clock_cycle++;

        std::set<int> bootstrapping_ids_to_remove;

        for (auto &[id, time_left] : bootstrapping_operations)
        {
            time_left--;
            if (time_left == 0)
            {
                bootstrapping_ids_to_remove.insert(id);
            }
        }

        for (auto id : bootstrapping_ids_to_remove)
        {
            bootstrapping_operations.erase(id);
        }

        std::set<int> running_ids_to_remove;

        for (auto &[id, time_left] : running_operations)
        {
            time_left--;
            if (time_left == 0)
            {
                if (lgr_parser.operation_is_bootstrapped(id))
                {
                    bootstrapping_operations[id] = bootstrapping_latency;
                }
                running_ids_to_remove.insert(id);
            }
        }

        for (auto id : running_ids_to_remove)
        {
            running_operations.erase(id);
        }
    }
    clock_cycle += map_max_value(running_operations);
    // std::cout << "Clock cycle: " << clock_cycle << std::endl;
    solver_latency = clock_cycle;
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

    output_file << "FINISH_TIME( OP0) " << solver_latency << std::endl;

    output_file.close();
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cout << "Usage: " << argv[0] << " <dag_file> <lgr_file> <output_file>" << std::endl;
        return 1;
    }

    std::string dag_file_path = std::string(argv[1]);
    std::string lgr_file_path = std::string(argv[2]);
    std::string output_file_path = std::string(argv[3]);

    ListScheduler list_scheduler = ListScheduler(dag_file_path, lgr_file_path);

    list_scheduler.generate_child_ids();
    list_scheduler.update_all_ESTs_and_LSTs();
    list_scheduler.update_all_ranks();
    list_scheduler.create_schedule();
    list_scheduler.generate_start_times_and_solver_latency(1);
    list_scheduler.write_lgr_like_format(output_file_path);

    return 0;
}
