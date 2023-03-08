#include "operation.h"

Operation::Operation(OperationType type, int id) : type{type}, id{id} {}

int Operation::get_earliest_end_time(const LatencyMap &latencies) const
{
    auto pre_bootstrap_latency = latencies.at(type);
    auto bootstrap_latency = is_bootstrapped()
                                 ? latencies.at(OperationType::BOOT)
                                 : 0;

    auto total_latency = pre_bootstrap_latency + bootstrap_latency;

    return earliest_start_time + total_latency;
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
    return parent->bootstrap_children.count(shared_from_this());
}

void Operation::update_earliest_start_time(const LatencyMap &latencies)
{
    auto bootstrap_latency = latencies.at(OperationType::BOOT);
    for (auto parent : parent_ptrs)
    {
        int parent_latency = latencies.at(parent->type);
        if (receives_bootstrapped_result_from(parent))
        {
            parent_latency += bootstrap_latency;
        }
        int possible_est =
            parent->earliest_start_time +
            parent_latency;
        if (possible_est > earliest_start_time)
        {
            earliest_start_time = possible_est;
        }
    }
}

void Operation::update_latest_start_time(const LatencyMap &latencies, const int earliest_possible_program_end_time)
{
    latest_start_time = std::numeric_limits<decltype(latest_start_time)>::max();

    if (child_ptrs.empty())
    {
        latest_start_time =
            earliest_possible_program_end_time - latencies.at(type);
    }
    else
    {
        auto bootstrap_latency = latencies.at(OperationType::BOOT);
        for (auto child : child_ptrs)
        {
            int child_latency = latencies.at(child->type);
            if (child->is_bootstrapped())
            {
                child_latency += bootstrap_latency;
            }
            int possible_lst =
                child->latest_start_time -
                child_latency;
            if (possible_lst < latest_start_time)
            {
                latest_start_time = possible_lst;
            }
        }
    }
}

// bool Operation::bootstraps_on_same_core_as(const OperationPtr &op)
// {
//     return op1->core_num == op2->core_num;
// }