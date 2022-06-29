import sys

bootstrapping_latency = 12


class SolutionValidator:
    def __init__(
        self,
        operation_types,
        operations,
        dependencies,
        bootstrapping_paths,
        solver_latency,
        bootstrapped_operations,
        used_bootstrap_limited_model,
        bootstrapping_start_times,
        cores_used_to_bootstrap,
    ):
        self.operation_types = operation_types
        self.dependencies = dependencies
        self.bootstrapping_paths = bootstrapping_paths
        self.uncompleted_operations = operations
        self.bootstrapped_operations = bootstrapped_operations
        self.solver_latency = solver_latency
        self.used_bootstrap_limited_model = used_bootstrap_limited_model
        self.bootstrapping_start_times = bootstrapping_start_times
        self.cores_used_to_bootstrap = cores_used_to_bootstrap
        self.running_operations = {}
        self.bootstrapping_queue = []
        self.bootstrapping_operations = {}
        self.clock_cycle = -1

    def validate_solution(self):
        self.bootstrapping_constraints_are_met()

        while self.program_is_not_complete():
            self.clock_cycle += 1

            self.bootstrapping_operations = self.decrement_cycles_left_for(
                self.bootstrapping_operations
            )
            self.running_operations = self.decrement_cycles_left_for(
                self.running_operations
            )

            self.bootstrapping_operations = self.remove_finished_operations(
                self.bootstrapping_operations
            )
            self.add_necessary_running_operations_to_bootstrapping_queue()
            self.running_operations = self.remove_finished_operations(
                self.running_operations
            )
            self.start_bootstrapping_ready_operations_from_queue()

            ready_operations = self.get_ready_operations()
            self.start_running_ready_operations(ready_operations)

            # print(self.clock_cycle)
            # print(self.uncompleted_operations)
            # print(self.running_operations)
            # print(self.bootstrapping_queue)
            # print(self.bootstrapping_operations)

        if self.clock_cycle != self.solver_latency:
            print("Error: The latencies mismatch")
            print("Solver latency: %d" % self.solver_latency)
            print("Calculated latency: %d" % self.clock_cycle)
        else:
            print("ILP model results are correct")
            sys.exit(0)

    def bootstrapping_constraints_are_met(self):
        for (operation, path_list) in self.bootstrapping_paths.items():
            for path in path_list:
                path_satisfied = False
                for bootstrapped_operation in self.bootstrapped_operations:
                    if bootstrapped_operation in path:
                        path_satisfied = True
                        break
                if not path_satisfied:
                    print("Error: The following path was not satsified")
                    print(path)
                    sys.exit(0)

    def program_is_not_complete(self):
        return (
            len(self.uncompleted_operations) > 0
            or len(self.running_operations) > 0
            or len(self.bootstrapping_queue) > 0
            or len(self.bootstrapping_operations) > 0
        )

    def decrement_cycles_left_for(self, operation_dict):
        for (operation, cycles_left) in operation_dict.items():
            operation_dict[operation] = cycles_left - 1
        return operation_dict

    def remove_finished_operations(self, operation_dict):
        return {
            operation: cycles_left
            for (operation, cycles_left) in operation_dict.items()
            if cycles_left > 0
        }

    def finish_operation_or_begin_bootstrapping(self, operation):
        if operation not in self.bootstrapped_operations:
            del self.running_operations[operation]
        else:

            del self.running_operations[operation]

    def add_necessary_running_operations_to_bootstrapping_queue(self):
        self.bootstrapping_queue += [
            operation
            for (operation, cycles_left) in self.running_operations.items()
            if operation in self.bootstrapped_operations and cycles_left == 0
        ]

    def start_bootstrapping_ready_operations_from_queue(self):
        if self.used_bootstrap_limited_model == "True":
            operations_to_remove = []
            for operation in self.bootstrapping_queue:
                if self.clock_cycle_matches_operation_bootstrapping_start_time(
                    operation
                ):
                    [
                        core_is_available,
                        bootstrapping_operation,
                    ] = self.check_if_operation_bootstrapping_core_is_available(
                        operation
                    )
                    if core_is_available:
                        self.bootstrapping_operations[operation] = bootstrapping_latency
                        operations_to_remove.append(operation)
                    else:
                        self.print_bootstrapping_core_is_not_available_error(
                            operation, bootstrapping_operation
                        )
                        sys.exit(0)
                elif self.clock_cycle_is_later_than_operation_bootstrapping_start_time(
                    operation
                ):
                    self.print_missed_bootstrapping_deadline_error(operation)
                    sys.exit(0)
            for operation in operations_to_remove:
                self.bootstrapping_queue.remove(operation)
        else:
            for operation in self.bootstrapping_queue:
                self.bootstrapping_operations[operation] = bootstrapping_latency
            self.bootstrapping_queue = []

    def clock_cycle_matches_operation_bootstrapping_start_time(self, operation):
        return (
            self.clock_cycle
            == self.bootstrapping_start_times[
                self.bootstrapped_operations.index(operation)
            ]
        )

    def clock_cycle_is_later_than_operation_bootstrapping_start_time(self, operation):
        return (
            self.clock_cycle
            > self.bootstrapping_start_times[
                self.bootstrapped_operations.index(operation)
            ]
        )

    def check_if_operation_bootstrapping_core_is_available(self, operation):
        for bootstrapping_operation in self.bootstrapping_operations:
            if self.operations_bootstrap_on_same_core(
                operation, bootstrapping_operation
            ):
                return [False, bootstrapping_operation]
        return [True, None]

    def operations_bootstrap_on_same_core(self, op1, op2):
        return (
            self.cores_used_to_bootstrap[self.bootstrapped_operations.index(op1)]
            == self.cores_used_to_bootstrap[self.bootstrapped_operations.index(op2)]
        )

    def print_bootstrapping_core_is_not_available_error(
        self, operation, bootrsapping_operation
    ):
        print(
            "Error: Bootstrapping operation %s was not completed before operation %s started bootstrapping on core %d"
            % (
                bootrsapping_operation,
                operation,
                self.cores_used_to_bootstrap[
                    self.bootstrapped_operations.index(operation)
                ],
            )
        )

    def print_missed_bootstrapping_deadline_error(self, operation):
        print("Error: Bootstrapping operation %s was did not start on time" % operation)

    def get_ready_operations(self):
        ready_operations = {
            operation: operation_type
            for (operation, operation_type) in self.uncompleted_operations.items()
            if self.operation_is_ready(operation)
        }
        return ready_operations

    def operation_is_ready(self, operation):
        if operation in self.dependencies:
            for dependency in self.dependencies[operation]:
                if (
                    dependency in self.uncompleted_operations
                    or dependency in self.running_operations
                    or dependency in self.bootstrapping_queue
                    or dependency in self.bootstrapping_operations
                ):
                    return False
        return True

    def start_running_ready_operations(self, ready_operations):
        for (operation, operation_type) in ready_operations.items():
            self.running_operations[operation] = int(
                self.operation_types[operation_type][1]
            )
            del self.uncompleted_operations[operation]