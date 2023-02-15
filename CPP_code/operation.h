#ifndef operation_INCLUDED_
#define operation_INCLUDED_

#include "operation_type.h"

#include <vector>
#include <memory>
#include <string>
#include <map>
#include <unordered_set>
#include <iostream>

class Operation;

using OperationPtr = std::shared_ptr<Operation>;
using OpVector = std::vector<OperationPtr>;
using OpSet = std::unordered_set<OperationPtr>;

using LatencyMap = std::map<OperationType::Type, int>;

struct Operation : public std::enable_shared_from_this<Operation>
{
    Operation(OperationType type, int id);

    OperationType type;
    int id;
    OpVector parent_ptrs;
    std::vector<int> constant_parent_ids;
    OpSet child_ptrs;
    OpSet bootstrap_children;
    int start_time;
    int bootstrap_start_time = 0;
    int core_num = 0;
    double bootstrap_urgency;
    int num_unsatisfied_segments;
    std::vector<size_t> segment_indexes;

    int get_earliest_end_time(const LatencyMap &) const;
    int get_slack() const;

    bool is_bootstrapped() const;
    bool has_no_parent_operations() const;
    bool parents_meet_urgency_criteria() const;
    bool has_multiplication_child() const;
    // bool bootstraps_on_the_same_core_as(const OperationPtr &);
    bool receives_bootstrapped_result_from(const OperationPtr &);

    void update_earliest_start_time(const LatencyMap &);
    void update_latest_start_time(const LatencyMap &, const int);

private:
    int earliest_start_time;
    int latest_start_time;
};

#endif