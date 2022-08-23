#include "simulated_annealing_scheduler.h"

void SimulatedAnnealingScheduler::create_schedule()
{
    int minimum_schedule_latency = numeric_limits<int>::max();
    auto min_schedule = get_init_schedule();
    auto schedule = min_schedule;

    for (auto i = 0; i < num_iterations; i++)
    {
        double temperature = 1 - ((double)i + 1) / num_iterations;

        auto new_schedule = get_random_neighbor();

        if (new_schedule_should_replace_schedule(new_schedule, schedule, temperature))
        {
            schedule = new_schedule;
            if (get_schedule_latency(schedule) < minimum_schedule_latency)
            {
                minimum_schedule_latency = get_schedule_latency(schedule);
                min_schedule = schedule;
            }
        }
    }
}

OperationList SimulatedAnnealingScheduler::get_init_schedule()
{
    auto list_scheduler = ListScheduler(dag_file_path, "NULL", num_cores);
    list_scheduler.perform_list_scheduling();
    return list_scheduler.get_operations();
}

OperationList SimulatedAnnealingScheduler::get_random_neighbor()
{
    get
}

bool SimulatedAnnealingScheduler::new_schedule_should_replace_schedule(OperationList new_schedule, OperationList schedule, double temperature)
{
    return false;
}