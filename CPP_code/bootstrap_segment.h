#ifndef bootstrap_segment_INCLUDED_
#define bootstrap_segment_INCLUDED_

#include "operation.h"
#include "shared_utils.h"

struct BootstrapPair
{
    OperationPtr parent;
    OperationPtr child;

    bool operator==(const BootstrapPair &other) const
    {
        return parent == other.parent && child == other.child;
    }

    struct Hash
    {
        auto operator()(const BootstrapPair &p) const
        {
            return std::hash<OperationPtr>()(p.parent) + std::hash<OperationPtr>()(p.child);
        }
    };
};

using BootstrapPairSet = std::unordered_set<BootstrapPair, BootstrapPair::Hash>;
using BootstrapPairIndexesMap = std::unordered_map<BootstrapPair, std::unordered_set<size_t>, BootstrapPair::Hash>;

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
    void update_satisfied_status(const BootstrapMode);
    bool is_satisfied() const;
    bool is_alive() const;
    bool relies_on_bootstrap_pair(const OperationPtr &, const OperationPtr &) const;
    BootstrapPairSet get_currently_satisfying_pairs() const;

private:
    void update_satisfied_status_in_complete_mode();
    void update_satisfied_status_in_selective_mode();
    OpVector segment;
    bool satisfied_status = false;
};

#endif