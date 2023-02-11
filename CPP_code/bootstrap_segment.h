#include "operation.h"
#include "shared_utils.h"

class BootstrapSegment
{
public:
    // OpVector::iterator begin();
    // OpVector::iterator end();
    OpVector::iterator begin() const;
    OpVector::iterator end() const;

    void add(const OperationPtr &);

    size_t size() const;

    OperationPtr operation_at(const size_t &) const;
    bool is_satisfied(const BootstrapMode &) const;
    bool is_alive(const BootstrapMode &) const;
    bool relies_on_bootstrap_pair(const OperationPtr &, const OperationPtr &) const;

private:
    bool is_satisfied_in_complete_mode() const;
    bool is_satisfied_in_selective_mode() const;
    OpVector segment;
};