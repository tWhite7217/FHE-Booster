#include "list_scheduling.h"

#include <limits>

class SimulatedAnnealingScheduler
{
public:
    void create_schedule();

private:
    size_t num_iterations;

    OperationList get_init_schedule();
    OperationList get_random_neighbor();
    bool new_schedule_should_replace_schedule(OperationList new_schedule, OperationList schedule, double temperature);
};