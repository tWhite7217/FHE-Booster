#include "bootstrap_segment.h"

void BootstrapSegment::add(const OperationPtr &op)
{
    segment.push_back(op);
}

size_t BootstrapSegment::size() const
{
    return segment.size();
}

OperationPtr BootstrapSegment::operation_at(const size_t &i) const
{
    return segment.at(i);
}

bool BootstrapSegment::is_satisfied(const BootstrapMode &mode) const
{
    switch (mode)
    {
    case BootstrapMode::COMPLETE:
        return is_satisfied_in_complete_mode();
    case BootstrapMode::SELECTIVE:
        return is_satisfied_in_selective_mode();
    }
}

bool BootstrapSegment::is_satisfied_in_complete_mode() const
{

    for (auto operation : segment)
    {
        if (operation->is_bootstrapped())
        {
            return true;
        }
    }
    return false;
}

bool BootstrapSegment::is_satisfied_in_selective_mode() const
{
    for (auto i = 0; i < segment.size() - 1; i++)
    {
        auto parent = segment[i];
        auto child = segment[i + 1];
        if (child->receives_bootstrapped_result_from(parent))
        {
            return true;
        }
    }
    return false;
}

bool BootstrapSegment::is_alive(const BootstrapMode &mode) const
{
    auto first_operation = segment.front();
    return !is_satisfied(mode) &&
           first_operation->parents_meet_urgency_criteria();
}

bool BootstrapSegment::relies_on_bootstrap_pair(const OperationPtr &parent, const OperationPtr &child) const
{
    for (auto i = 0; i < segment.size() - 1; i++)
    {
        auto other_parent = segment[i];
        auto other_child = segment[i + 1];
        if (other_parent != parent || other_child != child)
        {
            if (other_parent->get_bootstrap_children().count(other_child))
            {
                return false;
            }
        }
    }
    return true;
}