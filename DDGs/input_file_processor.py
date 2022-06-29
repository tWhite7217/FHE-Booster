import sys
from custom_ddg_format_parser import InputParser

bootstrapping_latency = 12
bootstrapping_path_length = 3
num_bootstrapping_cores = 1

input_filename = sys.argv[1]
output_filename = sys.argv[2]

allow_bootstrapping_to_only_some_children = sys.argv[3] == "True"

input_parser = InputParser(bootstrapping_path_length, False)
input_parser.parse_input(input_filename)

operation_types = input_parser.operation_types
operations = input_parser.operations
dependencies = input_parser.dependencies
bootstrapping_paths = input_parser.bootstrapping_paths

outputfile = open(output_filename, "w")

for operation_type in operation_types:
    outputfile.write(
        "T%d %s\n"
        % (operation_types[operation_type][0], operation_types[operation_type][1])
    )

outputfile.write("~\n")

for operation in operations:
    outputfile.write("%s\n" % operation)

outputfile.write("~\n")

for operation in operations:
    operation_type = operations[operation]
    operation_type_num = operation_types[operation_type][0]
    for i in range(len(operation_types)):
        if i == operation_type_num:
            outputfile.write("1 ")
        else:
            outputfile.write("0 ")
    outputfile.write("\n")

outputfile.write("~\n")

for child in dependencies:
    for parent in dependencies[child]:
        outputfile.write("%s %s\n" % (parent, child))

outputfile.write("~\n")

outputfile.write("%d\n" % bootstrapping_latency)

outputfile.write("~\n")

for (operation, path_list) in bootstrapping_paths.items():
    for path in path_list:
        constraint_string = ""
        for i in range(len(path) - 1):
            if allow_bootstrapping_to_only_some_children:
                constraint_string += "BOOTSTRAPPED(%s, %s) + " % (
                    path[i][2:],
                    path[i + 1][2:],
                )
            else:
                constraint_string += "BOOTSTRAPPED(%s) + " % (path[i][2:])
        constraint_string = constraint_string[:-2] + ">= 1;\n"
        outputfile.write(constraint_string)

outputfile.write("~\n")

if num_bootstrapping_cores > 0:
    outputfile.write("C1..C%d\n" % num_bootstrapping_cores)

outputfile.close()