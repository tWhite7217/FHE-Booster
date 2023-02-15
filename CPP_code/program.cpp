#include "program.h"

Program::Program(const ConstructorInput &in)
{
    InputParser input_parser;

    *this = *input_parser.parse_dag_file_with_bootstrap_file(in.dag_filename, in.bootstrap_filename);

    if (!in.segments_filename.empty())
    {
        bootstrap_segments = input_parser.parse_segments_file(in.segments_filename);
    }

    if (!in.latency_filename.empty())
    {
        latencies = input_parser.parse_latency_file(in.latency_filename);
    }
}

OpVector::const_iterator Program::begin() const { return operations.begin(); };
OpVector::const_iterator Program::end() const { return operations.end(); };
size_t Program::size() const { return operations.size(); };

OperationPtr Program::get_operation_ptr_from_id(const int id) const
{
    if (id < 1 || id > operations.size())
    {
        throw std::runtime_error("Invalid operation id");
    }
    return operations.at(id - 1);
}

int Program::get_latency_of(const OperationType::Type type) const
{
    return latencies.at(type);
};

void Program::update_ESTs_and_LSTs()
{
    // auto operations_in_topological_order = get_operations_in_topological_order();
    // TGFF puts operations in topological order so we do not need to do that here
    auto operations_in_topological_order = operations;

    int earliest_program_end_time = 0;
    for (auto operation : operations_in_topological_order)
    {
        operation->update_earliest_start_time(latencies);
        auto earliest_operation_end_time = operation->get_earliest_end_time(latencies);
        if (earliest_operation_end_time > earliest_program_end_time)
        {
            earliest_program_end_time = earliest_operation_end_time;
        }
    }

    // int  = get_earliest_possible_end_time();

    auto operations_in_reverse_topological_order = operations_in_topological_order;
    std::reverse(operations_in_reverse_topological_order.begin(), operations_in_reverse_topological_order.end());

    for (auto operation : operations_in_reverse_topological_order)
    {
        operation->update_latest_start_time(latencies, earliest_program_end_time);
    }
}

int Program::get_maximum_slack() const
{
    int max = 0;
    for (auto &operation : operations)
    {
        auto slack = operation->get_slack();
        if (slack > max)
        {
            max = slack;
        }
    }
    return max;
}

// void Program::add_segment_index_info_to_operations()
// {
//     auto segment_index = 0;
//     for (const auto &segment : bootstrap_segments)
//     {
//         for (auto &operation : segment)
//         {
//             operation->segment_indexes.push_back(segment_index);
//         }
//         segment_index++;
//     }
// }

bool Program::bootstrap_segments_are_satisfied() const
{
    return find_unsatisfied_bootstrap_segment_index() == -1;
}

// bool bootstrap_segments_are_satisfied_for_selective_model(std::vector<OpVector> &bootstrap_segments)
// {
//     return find_unsatisfied_bootstrap_segment_index(bootstrap_segments, bootstrap_segment_is_satisfied_for_selective_model) == -1;
// }

int Program::find_unsatisfied_bootstrap_segment_index() const
{
    for (int i = 0; i < bootstrap_segments.size(); i++)
    {
        if (!bootstrap_segments[i].is_satisfied(mode))
        {
            return i;
        }
    }
    return -1;
}

void Program::add_operation(const OperationPtr &operation)
{
    operations.push_back(operation);
}

void Program::reset_bootstrap_set()
{
    for (auto operation : operations)
    {
        operation->bootstrap_children.clear();
    }
}

void Program::update_num_segments_for_every_operation()
{
    for (auto &operation : operations)
    {
        operation->num_unsatisfied_segments = 0;
    }

    for (auto segment : bootstrap_segments)
    {
        if (!segment.is_satisfied(mode))
        {
            for (auto &operation : segment)
            {
                operation->num_unsatisfied_segments++;
            }
        }
    }
}

int Program::get_maximum_num_segments() const
{
    int max = 0;

    for (auto &operation : operations)
    {
        auto num_segments = operation->num_unsatisfied_segments;
        if (num_segments > max)
        {
            max = num_segments;
        }
    }

    return max;
}

void Program::update_all_bootstrap_urgencies()
{
    for (const auto &operation : operations)
    {
        operation->bootstrap_urgency = -1;
    }

    auto count = 0;
    for (const auto &segment : bootstrap_segments)
    {
        if (segment.is_alive(mode))
        {
            count++;
            auto segment_size = segment.size();
            for (double i = 0; i < segment_size; i++)
            {
                segment.operation_at(i)->bootstrap_urgency =
                    std::max(segment.operation_at(i)->bootstrap_urgency,
                             (i + 1) / segment_size);
            }
        }
    }
    if (count == 0)
    {
        std::cout << "here" << std::endl;
    }
}

void Program::remove_unnecessary_bootstrap_pairs()
{
    int num_removed = 0;
    for (auto parent : operations)
    {
        auto child_it = parent->bootstrap_children.begin();
        while (child_it != parent->bootstrap_children.end())
        {
            auto child = *child_it;
            if (no_segment_relies_on_bootstrap_pair(parent, child))
            {
                child_it = parent->bootstrap_children.erase(child_it);
                num_removed++;
            }
            else
            {
                child_it++;
            }
        }
    }
    std::cout << "Removed " << num_removed << " unnecessary bootstrapped results." << std::endl;
}

bool Program::no_segment_relies_on_bootstrap_pair(const OperationPtr &parent, const OperationPtr &child)
{
    for (auto segment : bootstrap_segments)
    {
        if (segment.relies_on_bootstrap_pair(parent, child))
        {
            return false;
        }
    }
    return true;
}

void Program::write_lgr_info_to_file(const std::string &filename, int total_latency) const
{
    std::ofstream output_file(filename);

    output_file << "Objective value: " << total_latency << ".0" << std::endl;

    write_bootstrapping_set_to_file(output_file);

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

    output_file << "FINISH_TIME( OP0) " << total_latency << std::endl;

    output_file.close();
}

void Program::write_bootstrapping_set_to_file(const std::string &filename) const
{
    std::ofstream output_file(filename);

    write_bootstrapping_set_to_file(output_file);

    output_file.close();
}

void Program::write_bootstrapping_set_to_file(std::ofstream &file) const
{
    if (mode == BootstrapMode::COMPLETE)
    {
        write_bootstrapping_set_to_file_complete_mode(file);
    }
    else
    {
        write_bootstrapping_set_to_file_selective_mode(file);
    }
}

void Program::write_bootstrapping_set_to_file_complete_mode(std::ofstream &file) const
{
    for (auto operation : operations)
    {
        if (operation->is_bootstrapped())
        {
            file << "BOOTSTRAPPED( OP" << operation->id << ") 1" << std::endl;
        }
    }
}

void Program::write_bootstrapping_set_to_file_selective_mode(std::ofstream &file) const
{

    for (auto operation : operations)
    {
        for (auto child : operation->bootstrap_children)
        {
            file << "BOOTSTRAPPED( OP" << operation->id << ", OP" << child->id << ") 1" << std::endl;
        }
    }
}