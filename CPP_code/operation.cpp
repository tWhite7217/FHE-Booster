#include "operation.h"

Operation::Operation(OperationType type, int id) : type{type}, id{id} {}

OperationType Operation::get_type() const
{
    return type;
}

int Operation::get_earliest_start_time() const
{
    return earliest_start_time;
}

int Operation::get_slack()
{
    return latest_start_time - earliest_start_time;
}

int Operation::get_id()
{
    return id;
}

OpVector Operation::get_parent_ptrs()
{
    return parent_ptrs;
}

OpVector Operation::get_child_ptrs()
{
    return child_ptrs;
}

OpSet Operation::get_bootstrap_children()
{
    return bootstrap_children;
}

std::vector<int> Operation::get_constant_parent_ids()
{
    return constant_parent_ids;
}

double Operation::get_bootstrap_urgency()
{
    return bootstrap_urgency;
}

int Operation::get_num_unsatisfied_segments()
{
    return num_unsatisfied_segments;
}

int Operation::get_core_num()
{
    return core_num;
}

// int Operation::get_start_time();

    void Operation::set_bootstrap_urgency(double) {
        bootstrap_urgency = double;
    }

    void Operation::set_core_num(int);

bool Operation::is_bootstrapped()
{
    return !bootstrap_children.empty();
}

bool Operation::has_no_parents()
{
    return (parent_ptrs.size() == 0) &&
           (constant_parent_ids.size() == 0);
}

bool Operation::parents_meet_urgency_criteria()
{
    for (auto parent : parent_ptrs)
    {
        if ((parent->segment_indexes.size() > 0) && !parent->is_bootstrapped())
        {
            return false;
        }
    }
    return true;
}

bool Operation::has_multiplication_child()
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

void Operation::update_latest_start_time(const LatencyMap &latencies, const int &earliest_possible_program_end_time)
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