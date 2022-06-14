import sys
from DDGs.custom_ddg_format_parser import InputParser

bootstrapping_latency = 12
bootstrapping_path_length = 3
num_bootstrapping_cores = 1

input_filename = sys.argv[1]
results_filename = sys.argv[2]

input_parser = InputParser(bootstrapping_path_length, True)
input_parser.parse_input(input_filename)

operation_types = input_parser.operation_types
operations = input_parser.operations
dependencies = input_parser.dependencies
bootstrapping_paths = input_parser.bootstrapping_paths


results_file = open(results_filename, "r")
results_lines = results_file.readlines()
results_file.close()

bootstrapped_operations = results_lines[0].split(",")
solver_latency = int(results_lines[1])

for (operation, path_list) in bootstrapping_paths.items():
    for path in path_list:
        path_satisfied = False
        for bootstrapped_operation in bootstrapped_operations:
            if bootstrapped_operation in path:
                path_satisfied = True
                break
        if not path_satisfied:
            print("Error: The following path was not satsified")
            print(path)
            sys.exit(0)


clock_cycle = -1
uncompleted_operations = operations.copy()
running_operations = {}

while not (len(uncompleted_operations) == 0 and len(running_operations) == 0):
    for operation in running_operations:
        running_operations[operation] -= 1

    running_operations = {
        operation: cycles_left
        for (operation, cycles_left) in running_operations.items()
        if cycles_left > 0
    }

    ready_operations = {}
    for (operation, operation_type) in uncompleted_operations.items():
        ready = True
        if operation in dependencies:
            for dependency in dependencies[operation]:
                if (
                    dependency in uncompleted_operations
                    or dependency in running_operations
                ):
                    ready = False
                    break
        if ready:
            ready_operations[operation] = operation_type

    for (operation, operation_type) in ready_operations.items():
        running_operations[operation] = int(operation_types[operation_type][1])
        if operation in bootstrapped_operations:
            running_operations[operation] += bootstrapping_latency
        del uncompleted_operations[operation]

    clock_cycle += 1

if clock_cycle != solver_latency:
    print("Error: The latencies mismatch")
    print("Solver latency: %d" % solver_latency)
    print("Calculated latency: %d" % clock_cycle)
else:
    print("ILP results are correct")