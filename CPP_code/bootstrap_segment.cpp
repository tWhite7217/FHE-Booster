#include "bootstrap_segment.h"

OpVector::iterator BootstrapSegment::begin() { return segment.begin(); }
OpVector::iterator BootstrapSegment::end() { return segment.end(); }

OpVector::const_iterator BootstrapSegment::begin() const { return segment.begin(); }
OpVector::const_iterator BootstrapSegment::end() const { return segment.end(); }

void BootstrapSegment::print() const
{
    std::ostringstream seg_stream;
    for (const auto &operation : segment)
    {
        seg_stream << operation->id << ",";
    }
    auto seg_string = seg_stream.str();
    seg_string.pop_back();
    std::cout << seg_string << std::endl;
}

void BootstrapSegment::add(const OperationPtr &op)
{
    segment.push_back(op);
}

void BootstrapSegment::remove_last_operation()
{
    segment.pop_back();
}

size_t BootstrapSegment::size() const
{
    return segment.size();
}

OperationPtr BootstrapSegment::operation_at(const size_t i) const
{
    return segment.at(i);
}

OperationPtr BootstrapSegment::first_operation() const { return segment.front(); }
OperationPtr BootstrapSegment::last_operation() const { return segment.back(); }

void BootstrapSegment::update_satisfied_status(const BootstrapMode mode)
{
    if (mode == BootstrapMode::SELECTIVE)
    {
        update_satisfied_status_in_selective_mode();
    }
    else
    {
        update_satisfied_status_in_complete_mode();
    }
}

void BootstrapSegment::update_satisfied_status_in_complete_mode()
{
    for (auto operation : segment)
    {
        if (operation->is_bootstrapped())
        {
            satisfied_status = true;
            return;
        }
    }
    satisfied_status = false;
}

void BootstrapSegment::update_satisfied_status_in_selective_mode()
{
    for (size_t i = 0; i < segment.size() - 1; i++)
    {
        auto parent = segment[i];
        auto child = segment[i + 1];
        if (child->receives_bootstrapped_result_from(parent))
        {
            satisfied_status = true;
            return;
        }
    }
    satisfied_status = false;
}

bool BootstrapSegment::is_satisfied() const
{
    return satisfied_status;
}

bool BootstrapSegment::is_alive() const
{
    auto first_operation = segment.front();
    return !satisfied_status &&
           first_operation->parents_meet_urgency_criteria();
}

bool BootstrapSegment::relies_on_bootstrap_pair(const OperationPtr &parent, const OperationPtr &child) const
{
    for (size_t i = 0; i < segment.size() - 1; i++)
    {
        auto other_parent = segment[i];
        auto other_child = segment[i + 1];
        if (other_parent != parent || other_child != child)
        {
            if (other_parent->bootstrap_children.contains(other_child))
            {
                return false;
            }
        }
    }
    return true;
}

BootstrapPairSet BootstrapSegment::get_currently_satisfying_pairs() const
{
    BootstrapPairSet currently_satisfying_pairs;
    for (size_t i = 0; i < segment.size() - 1; i++)
    {
        auto parent = segment[i];
        auto child = segment[i + 1];
        if (parent->bootstrap_children.contains(child))
        {
            currently_satisfying_pairs.insert({parent, child});
        }
    }
    return currently_satisfying_pairs;
}