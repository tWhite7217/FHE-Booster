import re


class InputParser:
    operation_types = {}
    operations = {}
    dependencies = {}
    bootstrapping_paths = {}

    def __init__(self, bootstrapping_path_length, is_for_validation):
        self.bootstrapping_path_length = bootstrapping_path_length
        self.is_for_validation = is_for_validation

    def parse_input(self, filename):
        inputfile = open(filename)
        lines = inputfile.readlines()
        inputfile.close()

        self.parse_lines(lines)

        if self.is_for_validation:
            self.create_bootstrapping_paths_for_validation()
        else:
            self.create_bootstrapping_paths()

    def parse_lines(self, lines):
        phase = 0
        for line in lines:
            line = self.get_string_array_from_line(line)

            if line[0] == "":
                return

            if line[0] == "~":
                phase = 1
            elif phase == 0:
                self.parse_operation_type(line)
            else:
                self.parse_operation_and_its_dependences(line)

    def parse_operation_type(self, line):
        self.operation_types[line[0]] = [len(self.operation_types), line[1]]

    def parse_operation_and_its_dependences(self, line):
        self.operations[line[0]] = line[1]
        dependence_list = line[2:]
        dependence_list = "".join(dependence_list)[1:-1].split(",")
        while "" in dependence_list:
            dependence_list.remove("")
        if len(dependence_list) > 0:
            self.dependencies[line[0]] = dependence_list

    def get_string_array_from_line(self, line):
        line = line.strip()
        line = re.sub(r"[\t ]+", " ", line)
        return line.split(" ")

    def create_bootstrapping_paths(self):
        for dependent_operation in self.dependencies:
            self.bootstrapping_paths[
                dependent_operation
            ] = self.create_bootstrapping_paths_helper(dependent_operation, 0)

    def create_bootstrapping_paths_helper(self, dependency, level):
        if level == self.bootstrapping_path_length:
            return [[dependency]]
        elif dependency not in self.dependencies:
            return [[]]

        paths_to_return = []
        for next_dependency in self.dependencies[dependency]:
            returned_paths = self.create_bootstrapping_paths_helper(
                next_dependency, level + 1
            )
            paths_to_return += [
                path if level == 0 else path + [dependency]
                for path in returned_paths
                if len(path) > 0
            ]
        return paths_to_return

    def create_bootstrapping_paths_for_validation(self):
        all_backward_paths = []
        for dependent_operation in self.dependencies:
            paths_from_dependent_operation = self.depth_first_search(
                [dependent_operation], []
            )
            all_backward_paths += paths_from_dependent_operation
        relevant_backward_paths = [
            path
            for path in all_backward_paths
            if len(path) == self.bootstrapping_path_length + 1
        ]
        for backward_path in relevant_backward_paths:
            path_to_add = list(reversed(backward_path[1:]))
            if backward_path[0] not in self.bootstrapping_paths:
                self.bootstrapping_paths[backward_path[0]] = [path_to_add]
            else:
                self.bootstrapping_paths[backward_path[0]] += [path_to_add]

    def depth_first_search(self, path, paths):
        current_end_of_path = path[-1]
        if current_end_of_path in self.dependencies:
            for dependency in self.dependencies[current_end_of_path]:
                new_path = path + [dependency]
                paths = self.depth_first_search(new_path, paths)
        else:
            paths += [path]
        return paths
