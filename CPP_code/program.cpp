#include "program.h"

Program::Program(const ConstructorInput &in)
{
    FileParser file_parser(std::ref(*this));

    file_parser.parse_dag_file_with_bootstrap_file(in.dag_filename, in.bootstrap_filename);

    if (!in.segments_filename.empty())
    {
        std::function<void()> parse_segs_func = [&file_parser, in]()
        { file_parser.parse_segments_file(in.segments_filename); };
        utl::perform_func_and_print_execution_time(
            parse_segs_func, "Parsing segments file");
    }

    if (!in.latency_filename.empty())
    {
        file_parser.parse_latency_file(in.latency_filename);
    }
}

OpVector::const_iterator Program::begin() const { return operations.begin(); };
OpVector::const_iterator Program::end() const { return operations.end(); };
size_t Program::size() const { return operations.size(); };

OperationPtr Program::get_operation_ptr_from_id(const size_t id) const
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

void Program::add_operation(const OperationPtr &operation)
{
    operations.push_back(operation);
}

void Program::set_bootstrap_segments(const std::vector<BootstrapSegment> &segments)
{
    bootstrap_segments = segments;
}

void Program::reset_bootstrap_set()
{
    for (auto operation : operations)
    {
        operation->bootstrap_children.clear();
    }
}

bool Program::has_unsatisfied_bootstrap_segments() const
{
    return unsatisfied_bootstrap_segment_indexes.size() > 0;
}

void Program::initialize_unsatisfied_segment_indexes()
{
    for (size_t i = 0; i < bootstrap_segments.size(); i++)
    {
        unsatisfied_bootstrap_segment_indexes.insert(i);
    }
}

void Program::initialize_num_segments_for_every_operation()
{
    for (const auto &segment : bootstrap_segments)
    {
        for (auto &operation : segment)
        {
            operation->num_unsatisfied_segments++;
        }
    }
}

void Program::update_unsatisfied_segments_and_num_segments_for_every_operation()
{
    auto seg_index_it = unsatisfied_bootstrap_segment_indexes.begin();
    while (seg_index_it != unsatisfied_bootstrap_segment_indexes.end())
    {
        const auto seg_index = *seg_index_it;
        auto &segment = bootstrap_segments[seg_index];
        segment.update_satisfied_status(mode);
        if (segment.is_satisfied())
        {
            seg_index_it = unsatisfied_bootstrap_segment_indexes.erase(seg_index_it);
            for (const auto &operation : segment)
            {
                operation->num_unsatisfied_segments--;
            }
        }
        else
        {
            seg_index_it++;
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
        if (segment.is_alive())
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
    auto pairs_to_indexes_map = get_candidate_pairs_and_segment_indexes();

    int num_removed = 0;
    for (const auto &[candidate_pair, segment_indexes] : pairs_to_indexes_map)
    {
        if (no_segment_relies_on_bootstrap_pair(candidate_pair, segment_indexes))
        {
            auto &[parent, child] = candidate_pair;
            parent->bootstrap_children.erase(child);
            num_removed++;
        }
    }
    std::cout << "Removed " << num_removed << " unnecessary bootstrapped results." << std::endl;
}

BootstrapPairIndexesMap Program::get_candidate_pairs_and_segment_indexes()
{
    BootstrapPairIndexesMap candidate_pairs_map;
    BootstrapPairSet needed_pairs;

    for (size_t i = 0; i < bootstrap_segments.size(); i++)
    {
        auto satisfying_pairs = bootstrap_segments[i].get_currently_satisfying_pairs();

        if (satisfying_pairs.size() > 1)
        {
            for (auto &pair : satisfying_pairs)
            {
                candidate_pairs_map[pair].insert(i);
            }
        }
        else
        {
            needed_pairs.insert(satisfying_pairs.begin(), satisfying_pairs.end());
        }
    }

    for (const auto &pair : needed_pairs)
    {
        candidate_pairs_map.erase(pair);
    }

    return candidate_pairs_map;
}

bool Program::no_segment_relies_on_bootstrap_pair(const BootstrapPair &pair, const std::unordered_set<size_t> &segment_indexes)
{
    const auto &[parent, child] = pair;
    for (const auto i : segment_indexes)
    {
        if (bootstrap_segments[i].relies_on_bootstrap_pair(parent, child))
        {
            return false;
        }
    }
    return true;
}

void Program::convert_segments_to_selective()
{
    std::vector<BootstrapSegment> new_segments;
    for (const auto &segment : bootstrap_segments)
    {
        for (const auto &child : segment.last_operation()->child_ptrs)
        {
            new_segments.push_back(segment);
            new_segments.back().add(child);
        }
    }
    bootstrap_segments = new_segments;
}