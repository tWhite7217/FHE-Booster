#ifndef operation_INCLUDED_
#define operation_INCLUDED_

#include <vector>
#include <memory>
#include <string>
#include <map>
#include <set>
#include <iostream>

class Operation;

using OperationPtr = std::shared_ptr<Operation>;
using OpVector = std::vector<OperationPtr>;
using OpSet = std::set<OperationPtr>;

class OperationType
{
public:
    enum Type
    {
        ADD,
        SUB,
        MUL,
        BOOT
    };

    static const size_t num_types_except_bootstrap = BOOT;

    OperationType(){};
    OperationType(const Type &type) : type{type} {};
    OperationType(const std::string &type_string)
    {
        if (type_string == "ADD")
        {
            type = ADD;
        }
        else if (type_string == "SUB")
        {
            type = SUB;
        }
        else if (type_string == "MUL")
        {
            type = MUL;
        }
        else if (type_string == "BOOT")
        {
            type = BOOT;
        }
        std::cout << "Invalid operation type: " << type_string << std::endl;
        exit(1);
    };

    std::string to_string() const
    {
        switch (type)
        {
        case ADD:
            return "ADD";
        case SUB:
            return "SUB";
        case MUL:
            return "MUL";
        case BOOT:
            return "BOOT";
        }
        return "";
    };

    // bool operator=(Type other_type) { type = other_type; }

    // bool operator==(Type other_type) { return type == other_type; }
    // bool operator==(OperationType other_class) { return type == other_class.type; }
    // bool operator!=(Type other_type) { return !operator==(other_type); }
    // bool operator!=(OperationType other_class) { return !operator==(other_class); };

    operator Type() const { return type; }

private:
    Type type;
};

// std::ostream &operator<<(std::ostream &os, OperationType const &type)
// {
//     return os << type.to_string();
// }

using LatencyMap = std::map<OperationType::Type, int>;

struct Operation : public std::enable_shared_from_this<Operation>
{
    Operation(OperationType type, int id);

    OperationType type;
    int id;
    OpVector parent_ptrs;
    std::vector<int> constant_parent_ids;
    OpVector child_ptrs;
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
    bool has_no_parents() const;
    bool parents_meet_urgency_criteria() const;
    bool has_multiplication_child() const;
    // bool bootstraps_on_the_same_core_as(const OperationPtr &);
    bool receives_bootstrapped_result_from(const OperationPtr &);

    void update_earliest_start_time(const LatencyMap &);
    void update_latest_start_time(const LatencyMap &, const int &);

private:
    int earliest_start_time;
    int latest_start_time;
};

#endif