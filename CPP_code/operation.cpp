#include "operation.h"

Operation::Operation(OperationType type, int id) : type{type}, id{id} {}

int Operation::get_total_latency(const LatencyMap &latencies) const
{
    auto pre_bootstrap_latency = latencies.at(type);
    auto bootstrap_latency = is_bootstrapped()
                                 ? latencies.at(OperationType::BOOT)
                                 : 0;

    return pre_bootstrap_latency + bootstrap_latency;
}

int Operation::get_slack() const
{
    return latest_start_time - earliest_start_time;
}

bool Operation::is_bootstrapped() const
{
    return !bootstrap_children.empty();
}

bool Operation::has_no_parent_operations() const
{
    return (parent_ptrs.size() == 0);
}

bool Operation::parents_meet_urgency_criteria() const
{
    for (auto parent : parent_ptrs)
    {
        if ((parent->exists_on_some_segment) && !parent->is_bootstrapped())
        {
            return false;
        }
    }
    return true;
}

bool Operation::has_multiplication_child() const
{
    for (const auto &child : child_ptrs)
    {
        if (child->type == OperationType::MUL)
        {
            return true;
        }
    }
    return false;
}

bool Operation::receives_bootstrapped_result_from(const OperationPtr &parent)
{
    return parent->bootstrap_children.contains(shared_from_this());
}

void Operation::update_earliest_start_and_finish_times(const LatencyMap &latencies)
{
    earliest_start_time = 0;

    auto bootstrap_latency = latencies.at(OperationType::BOOT);
    for (auto parent : parent_ptrs)
    {
        earliest_start_time = std::max(earliest_start_time, parent->earliest_finish_time);
    }

    earliest_finish_time = earliest_start_time + get_total_latency(latencies);
}

void Operation::update_latest_start_time(const LatencyMap &latencies, const int earliest_possible_program_end_time)
{
    auto latest_finish_time = earliest_possible_program_end_time;

    for (auto child : child_ptrs)
    {
        latest_finish_time = std::min(latest_finish_time, child->latest_start_time);
    }

    latest_start_time = latest_finish_time - get_total_latency(latencies);
}

// bool Operation::bootstraps_on_same_core_as(const OperationPtr &op)
// {
//     return op1->core_num == op2->core_num;
// }