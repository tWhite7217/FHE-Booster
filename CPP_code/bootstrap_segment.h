#ifndef bootstrap_segment_INCLUDED_
#define bootstrap_segment_INCLUDED_

#include "operation.h"
#include "shared_utils.h"

class BootstrapSegment
{
public:
    // OpVector::iterator begin();
    // OpVector::iterator end();
    OpVector::const_iterator begin() const;
    OpVector::const_iterator end() const;

    void add(const OperationPtr &);
    void remove_last_operation();

    size_t size() const;

    OperationPtr operation_at(const size_t) const;
    OperationPtr first_operation() const;
    OperationPtr last_operation() const;
    bool is_satisfied(const BootstrapMode) const;
    bool is_alive(const BootstrapMode) const;
    bool relies_on_bootstrap_pair(const OperationPtr &, const OperationPtr &) const;

private:
    bool is_satisfied_in_complete_mode() const;
    bool is_satisfied_in_selective_mode() const;
    OpVector segment;
};

#endif