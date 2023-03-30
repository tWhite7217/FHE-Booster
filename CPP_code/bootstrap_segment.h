#ifndef bootstrap_segment_INCLUDED_
#define bootstrap_segment_INCLUDED_

#include "operation.h"
#include "shared_utils.h"

struct BootstrapPair
{
    OperationPtr parent;
    OperationPtr child;

    bool operator<(const BootstrapPair &other) const
    {
        if (parent->id == other.parent->id)
        {
            return child->id < other.child->id;
        }
        return parent->id < other.parent->id;
    }
};

using BootstrapPairSet = std::set<BootstrapPair>;
using BootstrapPairIndexesMap = std::map<BootstrapPair, std::unordered_set<size_t>>;

class BootstrapSegment
{
public:
    OpVector::iterator begin();
    OpVector::iterator end();
    OpVector::const_iterator begin() const;
    OpVector::const_iterator end() const;

    void print() const;

    void add(const OperationPtr &);
    void remove_last_operation();

    size_t size() const;

    OperationPtr operation_at(const size_t) const;
    OperationPtr first_operation() const;
    OperationPtr last_operation() const;
    OperationPtr last_multiplication() const;
    void update_satisfied_status(const BootstrapMode);
    bool is_satisfied() const;
    bool is_alive() const;
    bool relies_on_bootstrap_pair(const OperationPtr &, const OperationPtr &) const;
    BootstrapPairSet get_currently_satisfying_pairs() const;

    void set_last_mul(const OperationPtr &);

private:
    void update_satisfied_status_in_complete_mode();
    void update_satisfied_status_in_selective_mode();
    OpVector segment;
    bool satisfied_status = false;
    OperationPtr last_mul = nullptr;
};

#endif