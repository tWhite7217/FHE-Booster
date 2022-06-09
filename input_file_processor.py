import re

bootstrapping_latency = 12
bootstrapping_path_length = 3
num_bootstrapping_cores = 1

inputfile = open("unprocessed_input_file.txt")

my_text = inputfile.readlines()

operation_types = {}
operations = {}
dependencies = {}
bootstrapping_paths = {}


def create_bootstrapping_paths():
    for dependent_operation in dependencies:
        bootstrapping_paths[dependent_operation] = create_bootstrapping_paths_helper(
            dependent_operation, 0
        )


def create_bootstrapping_paths_helper(dependency, level):
    if level == bootstrapping_path_length:
        return [[dependency]]
    elif dependency not in dependencies:
        return [[]]

    paths_to_return = []
    for next_dependency in dependencies[dependency]:
        returned_paths = create_bootstrapping_paths_helper(next_dependency, level + 1)
        paths_to_return += [
            path if level == 0 else path + [dependency]
            for path in returned_paths
            if len(path) > 0
        ]
    return paths_to_return


phase = 0

for line in my_text:
    line = line.strip()
    line = re.sub(r"[\t ]+", " ", line)
    line = line.split(" ")

    if line[0] == "":
        continue

    if phase == 0:
        if line[0] == "~":
            phase = 1
        else:
            operation_types[line[0]] = [len(operation_types), line[1]]
    else:
        operations[line[0]] = line[1]

        dependence_list = line[2:]
        dependence_list = "".join(dependence_list)[1:-1].split(",")
        while "" in dependence_list:
            dependence_list.remove("")
        if len(dependence_list) > 0:
            dependencies[line[0]] = dependence_list

inputfile.close()

outputfile = open("processed_input_file.LDT", "w")

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

for dependent_operation in dependencies:
    for dependency in dependencies[dependent_operation]:
        outputfile.write("%s %s\n" % (dependent_operation, dependency))

outputfile.write("~\n")

outputfile.write("%d\n" % bootstrapping_latency)

outputfile.write("~\n")

if num_bootstrapping_cores > 0:
    outputfile.write("C1..C%d\n" % num_bootstrapping_cores)

outputfile.write("~\n")

create_bootstrapping_paths()

for (operation, path_list) in bootstrapping_paths.items():
    for path in path_list:
        constraint_string = ""
        for dependency in path:
            constraint_string += "BOOTSTRAPPED(%s) + " % (dependency[2:])
        constraint_string = constraint_string[:-2] + ">= 1;\n"
        outputfile.write(constraint_string)


outputfile.close()