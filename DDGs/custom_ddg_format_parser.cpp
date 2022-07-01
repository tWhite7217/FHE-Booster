#include <algorithm>

#include "custom_ddg_format_parser.h"

InputParser::InputParser(int bootstrapping_path_length, bool is_for_validation, bool allow_bootstrapping_to_some_children_only)
    : bootstrapping_path_length{bootstrapping_path_length},
      is_for_validation{is_for_validation},
      allow_bootstrapping_to_some_children_only{allow_bootstrapping_to_some_children_only} {}

std::map<std::string, int> InputParser::get_operation_type_to_latency_map() { return operation_type_to_latency_map; }
std::vector<Operation> InputParser::get_operations() { return operations; }
std::vector<std::vector<int>> InputParser::get_bootstrapping_paths() { return bootstrapping_paths; }

void InputParser::parse_input(std::string filename)
{
    std::fstream input_file;
    input_file.open(filename, std::ios::in);

    parse_lines(input_file);

    input_file.close();

    if (is_for_validation)
    {
        create_bootstrapping_paths_for_validation();
    }
    else
    {
        create_bootstrapping_paths();
    }

    if (!allow_bootstrapping_to_some_children_only)
    {
        remove_last_operation_from_bootstrapping_paths();
        remove_duplicate_bootstrapping_paths();
    }
}

Operation InputParser::get_operation_from_id(int id)
{
    if (id < 1 || id > operations.size())
    {
        throw std::runtime_error("Invalid operation id");
    }
    return operations[id - 1];
}

void InputParser::parse_lines(std::fstream &input_file)
{
    int phase = 0;
    std::string line;
    while (std::getline(input_file, line))
    {
        auto line_as_list = get_string_list_from_line(line);

        if (line_as_list[0] == "")
        {
            return;
        }

        if (line_as_list[0] == "~")
        {
            phase = 1;
        }
        else if (phase == 0)
        {
            parse_operation_type(line_as_list);
        }
        else
        {
            parse_operation_and_its_dependences(line_as_list);
        }
    }
}

void InputParser::parse_operation_type(std::vector<std::string> line)
{
    operation_type_to_latency_map[line[0]] = std::stoi(line[1]);
}

void InputParser::parse_operation_and_its_dependences(std::vector<std::string> line)
{
    std::string type = line[1];
    std::vector<int> parent_ids;
    for (int i = 2; i < line.size(); i++)
    {
        parent_ids.push_back(std::stoi(line[i]));
    }
    operations.push_back(Operation{type, parent_ids});
}

std::vector<std::string> InputParser::get_string_list_from_line(std::string line)
{
    std::vector<std::string> line_as_list;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, ','))
    {
        line_as_list.push_back(item);
    }
    return line_as_list;
}

void InputParser::create_bootstrapping_paths()
{
    for (auto operation_id = 1; operation_id <= operations.size(); operation_id++)
    {
        auto bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation_id, 0);
        bootstrapping_paths.insert(bootstrapping_paths.end(), bootstrapping_paths_to_add.begin(), bootstrapping_paths_to_add.end());
    }
}

std::vector<std::vector<int>> InputParser::create_bootstrapping_paths_helper(int operation_id, int level)
{
    if (level == bootstrapping_path_length)
    {
        return {{operation_id}};
    }
    else if (get_operation_from_id(operation_id).parent_ids.size() == 0)
    {
        return {};
    }

    std::vector<std::vector<int>> paths_to_return;
    for (auto parent_id : get_operation_from_id(operation_id).parent_ids)
    {
        auto paths_to_add = create_bootstrapping_paths_helper(parent_id, level + 1);
        for (auto path : paths_to_add)
        {
            path.push_back(operation_id);
            paths_to_return.push_back(path);
        }
    }
    return paths_to_return;
}

void InputParser::create_bootstrapping_paths_for_validation()
{
    std::vector<std::vector<int>> all_backward_paths;
    for (auto operation_id = 1; operation_id <= operations.size(); operation_id++)
    {
        auto backward_paths_from_operation = depth_first_search(
            {operation_id}, {});
        all_backward_paths.insert(all_backward_paths.end(), backward_paths_from_operation.begin(), backward_paths_from_operation.end());
    }

    for (auto &path : all_backward_paths)
    {
        if (path.size() == bootstrapping_path_length + 1)
        {
            std::reverse(path.begin(), path.end());
            bootstrapping_paths.push_back(path);
        }
    }
}

std::vector<std::vector<int>> InputParser::depth_first_search(std::vector<int> current_path, std::vector<std::vector<int>> backward_paths)
{
    auto current_end_of_path = current_path.back();
    if (get_operation_from_id(current_end_of_path).parent_ids.size() > 0)
    {
        for (auto parent_id : get_operation_from_id(current_end_of_path).parent_ids)
        {
            std::vector<int> new_path;
            std::copy(current_path.begin(), current_path.end(), std::back_inserter(new_path));
            new_path.push_back(parent_id);
            backward_paths = depth_first_search(new_path, backward_paths);
        }
    }
    else
    {
        backward_paths.push_back(current_path);
    }
    return backward_paths;
}

void InputParser::remove_last_operation_from_bootstrapping_paths()
{
    for (auto &path : bootstrapping_paths)
    {
        path.pop_back();
    }
}

void InputParser::remove_duplicate_bootstrapping_paths()
{
    if (bootstrapping_paths.size() <= 1)
    {
        return;
    }

    auto logical_end = std::unique(bootstrapping_paths.begin(), bootstrapping_paths.end(),
                                   [](auto &a, auto &b)
                                   { return std::equal(a.begin(), a.end(), b.begin()); });

    bootstrapping_paths.erase(logical_end,
                              bootstrapping_paths.end());
}