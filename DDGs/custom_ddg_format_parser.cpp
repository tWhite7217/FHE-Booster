#include <algorithm>

#include "custom_ddg_format_parser.h"

InputParser::InputParser(bool is_for_validation, bool allow_bootstrapping_to_some_children_only)
    : is_for_validation{is_for_validation},
      allow_bootstrapping_to_some_children_only{allow_bootstrapping_to_some_children_only} {}

std::map<std::string, int> InputParser::get_operation_type_to_latency_map() { return operation_type_to_latency_map; }
OperationList InputParser::get_operations() { return operations; }
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

    remove_redundant_bootstrapping_paths();
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
    operations.add(Operation{type, parent_ids, int(operations.size()) + 1});
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
        std::vector<std::vector<int>> bootstrapping_paths_to_add;
        if (operations.get(operation_id).type == "MUL")
        {
            bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation_id, 1, 0);
        }
        else
        {
            bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation_id, 0, 1);
        }
        bootstrapping_paths.insert(bootstrapping_paths.end(), bootstrapping_paths_to_add.begin(), bootstrapping_paths_to_add.end());
    }
}

std::vector<std::vector<int>> InputParser::create_bootstrapping_paths_helper(int operation_id, int num_multiplications, int num_additions)
{
    float path_cost = (num_multiplications + (float)num_additions / addition_divider);
    if (path_cost > bootstrapping_path_threshold)
    {
        return {{operation_id}};
    }
    else if (operations.get(operation_id).parent_ids.size() == 0)
    {
        return {};
    }

    std::vector<std::vector<int>> paths_to_return;
    for (auto parent_id : operations.get(operation_id).parent_ids)
    {
        std::vector<std::vector<int>> paths_to_add;
        if (operations.get(parent_id).type == "MUL")
        {
            paths_to_add = create_bootstrapping_paths_helper(parent_id, num_multiplications + 1, num_additions);
        }
        else
        {
            paths_to_add = create_bootstrapping_paths_helper(parent_id, num_multiplications, num_additions + 1);
        }
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

    std::vector<std::vector<int>> paths_above_threshold;
    for (auto &path : all_backward_paths)
    {
        if (get_path_cost(path) > bootstrapping_path_threshold)
        {
            std::reverse(path.begin(), path.end());
            paths_above_threshold.push_back(path);
        }
    }

    auto i = 0;
    for (auto path : paths_above_threshold)
    {
        path.pop_back();
        if (get_path_cost(path) <= bootstrapping_path_threshold)
        {
            bootstrapping_paths.push_back(paths_above_threshold[i]);
        }
        i++;
    }
}

float InputParser::get_path_cost(std::vector<int> path)
{
    int num_multiplications = 0;
    int num_additions = 0;
    for (auto operation_id : path)
    {
        if (operations.get(operation_id).type == "MUL")
        {
            num_multiplications++;
        }
        else
        {
            num_additions++;
        }
    }
    return num_multiplications + (float)num_additions / addition_divider;
}

std::vector<std::vector<int>> InputParser::depth_first_search(std::vector<int> current_path, std::vector<std::vector<int>> backward_paths)
{
    auto current_end_of_path = current_path.back();
    if (operations.get(current_end_of_path).parent_ids.size() > 0)
    {
        for (auto parent_id : operations.get(current_end_of_path).parent_ids)
        {
            std::vector<int> new_path;
            std::copy(current_path.begin(), current_path.end(), std::back_inserter(new_path));
            new_path.push_back(parent_id);
            backward_paths = depth_first_search(new_path, backward_paths);
        }
    }
    backward_paths.push_back(current_path);
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

    std::sort(bootstrapping_paths.begin(), bootstrapping_paths.end());

    auto logical_end = std::unique(bootstrapping_paths.begin(), bootstrapping_paths.end());

    bootstrapping_paths.erase(logical_end,
                              bootstrapping_paths.end());
}

void InputParser::remove_redundant_bootstrapping_paths()
{
    size_t min_path_length = std::numeric_limits<size_t>::max();
    for (auto path : bootstrapping_paths)
    {
        min_path_length = std::min(min_path_length, path.size());
    }

    std::vector<size_t> indices_to_remove;
    for (auto i = 0; i < bootstrapping_paths.size(); i++)
    {
        if (bootstrapping_paths[i].size() > min_path_length)
        {
            if (path_is_redundant(i))
            {
                indices_to_remove.push_back(i);
            }
        }
    }

    int num_redundant_paths = indices_to_remove.size();
    for (int i = num_redundant_paths - 1; i >= 0; i--)
    {
        bootstrapping_paths.erase(bootstrapping_paths.begin() + indices_to_remove[i]);
    }
}

bool InputParser::path_is_redundant(size_t path_index)
{
    auto test_path = bootstrapping_paths[path_index];
    auto test_path_size = test_path.size();
    for (auto i = 0; i < bootstrapping_paths.size(); i++)
    {
        auto path_i = bootstrapping_paths[i];
        auto path_i_size = path_i.size();
        auto size_difference = test_path_size - path_i_size;
        if (i != path_index && size_difference > 0)
        {
            if (larger_path_contains_smaller_path(test_path, path_i))
            {
                return true;
            }
        }
    }
    return false;
}

bool InputParser::larger_path_contains_smaller_path(std::vector<int> larger_path, std::vector<int> smaller_path)
{
    if (allow_bootstrapping_to_some_children_only)
    {
        for (auto i = 0; i < larger_path.size() - 1; i++)
        {
            for (auto j = 0; j < smaller_path.size() - 1; j++)
            {
                if (larger_path[i] != smaller_path[j] || larger_path[i + 1] != smaller_path[j + 1])
                {
                    return false;
                }
            }
        }
    }
    else
    {
        for (auto i = 0; i < smaller_path.size(); i++)
        {
            if (!vector_contains_element(larger_path, smaller_path[i]))
            {
                return false;
            }
        }
    }
    return true;
}
