import sys
from DDGs.custom_ddg_format_parser import InputParser
from solution_validator import SolutionValidator

bootstrapping_path_length = 3

input_filename = sys.argv[1]
results_filename = sys.argv[2]
used_bootstrap_limited_model = sys.argv[3]

input_parser = InputParser(bootstrapping_path_length, True)
input_parser.parse_input(input_filename)

results_file = open(results_filename, "r")
results_lines = results_file.readlines()
results_file.close()

bootstrapped_operations = results_lines[0].strip().split(",")
solver_latency = int(results_lines[1])
cores_used_to_bootstrap = []
bootstrapping_start_times = []
if used_bootstrap_limited_model == "True":
    cores_used_to_bootstrap = [
        int(core) for core in results_lines[2].strip().split(",")[:-1]
    ]
    bootstrapping_start_times = [
        int(start_time) for start_time in results_lines[3].strip().split(",")[:-1]
    ]

solution_validator = SolutionValidator(
    input_parser.operation_types,
    input_parser.operations,
    input_parser.dependencies,
    input_parser.bootstrapping_paths,
    solver_latency,
    bootstrapped_operations,
    used_bootstrap_limited_model,
    bootstrapping_start_times,
    cores_used_to_bootstrap
)

solution_validator.validate_solution()
