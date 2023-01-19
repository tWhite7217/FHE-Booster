#include "bootstrapping_path_generator.h"

BootstrappingPathGenerator::BootstrappingPathGenerator(OperationList operations, bool using_selective_model, int gained_levels)
    : operations{operations}, using_selective_model{using_selective_model}, gained_levels{gained_levels} {}

std::vector<OperationList> BootstrappingPathGenerator::get_bootstrapping_paths(std::string input_dag_file_path)
{
    std::string bootstrapping_file_path = input_dag_file_path.substr(0, input_dag_file_path.size() - 4) + "_bootstrapping_paths_" + std::to_string(gained_levels) + "_levels";
    if (using_selective_model)
    {
        bootstrapping_file_path += "_selective";
    }
    bootstrapping_file_path += ".txt";
    struct stat input_dag_file_info;
    struct stat output_bootstrapping_file_info;

    stat(input_dag_file_path.c_str(), &input_dag_file_info);
    auto result = stat(bootstrapping_file_path.c_str(), &output_bootstrapping_file_info);

    if (result == 0 && input_dag_file_info.st_mtime < output_bootstrapping_file_info.st_mtime)
    {
        auto input_file = std::ifstream(bootstrapping_file_path);
        std::cout << "Reading bootstrapping paths from file." << std::endl;
        return read_bootstrapping_paths(input_file, operations);
    }
    else
    {
        auto output_file = std::ofstream(bootstrapping_file_path);
        std::cout << "Generating bootstrapping paths." << std::endl;
        generate_bootstrapping_paths();
        write_paths_to_file(output_file);
        add_path_num_info_to_all_operations();
        return bootstrapping_paths;
    }
}

void BootstrappingPathGenerator::generate_bootstrapping_paths()
{
    create_raw_bootstrapping_paths();
    clean_raw_bootstrapping_paths();
}

void BootstrappingPathGenerator::create_raw_bootstrapping_paths()
{
    for (auto operation : operations)
    {
        std::vector<std::vector<OperationPtr>> bootstrapping_paths_to_add;
        if (operation->type == "MUL")
        {
            bootstrapping_paths_to_add = create_bootstrapping_paths_helper(operation, {}, 0);
        }
        bootstrapping_paths.insert(bootstrapping_paths.end(), bootstrapping_paths_to_add.begin(), bootstrapping_paths_to_add.end());
    }

    class comparator_class
    {
    public:
        bool operator()(std::vector<OperationPtr> path1, std::vector<OperationPtr> path2)
        {
            if (path1.back() == path2.back())
            {
                return path1.size() < path2.size();
            }
            else
            {
                return path1.back()->id < path2.back()->id;
            }
        }
    };

    auto starting_point_offset = 0;
    if (bootstrapping_paths.size() > 0)
    {
        auto current_starting_id_to_sort = bootstrapping_paths[0][0];
        for (auto i = 1; i < bootstrapping_paths.size(); i++)
        {
            if (bootstrapping_paths[i][0] != current_starting_id_to_sort)
            {
                std::sort(bootstrapping_paths.begin() + starting_point_offset, bootstrapping_paths.begin() + i, comparator_class());
                starting_point_offset = i;
                current_starting_id_to_sort = bootstrapping_paths[i][0];
            }
        }
    }
    else
    {
        std::cout << "This program has no bootstrapping paths." << std::endl;
    }
}

std::vector<std::vector<OperationPtr>> BootstrappingPathGenerator::create_bootstrapping_paths_helper(OperationPtr operation, std::vector<OperationPtr> path, int num_multiplications)
{
    if (operation->type == "MUL")
    {
        num_multiplications++;
    }
    path.push_back(operation);
    if (num_multiplications >= gained_levels && operation_has_multiplication_child(operation))
    {
        std::vector<OperationList> paths_to_return;
        for (auto child : operation->child_ptrs)
        {
            auto path_to_return = path;
            path_to_return.push_back(child);
            paths_to_return.push_back(path_to_return);
        }
        return paths_to_return;
    }
    else if (operation->child_ptrs.size() == 0)
    {
        return {};
    }

    std::vector<OperationList> paths_to_return;
    for (auto child : operation->child_ptrs)
    {
        std::vector<std::vector<OperationPtr>> paths_to_add;
        paths_to_add = create_bootstrapping_paths_helper(child, path, num_multiplications);
        paths_to_return.insert(paths_to_return.end(), paths_to_add.begin(), paths_to_add.end());
    }
    return paths_to_return;
}

void BootstrappingPathGenerator::clean_raw_bootstrapping_paths()
{
    remove_redundant_bootstrapping_paths();

    if (!using_selective_model)
    {
        remove_last_operation_from_bootstrapping_paths();
        remove_duplicate_bootstrapping_paths();
    }
}

void BootstrappingPathGenerator::remove_last_operation_from_bootstrapping_paths()
{
    for (auto &path : bootstrapping_paths)
    {
        path.pop_back();
    }
}

void BootstrappingPathGenerator::remove_duplicate_bootstrapping_paths()
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

void BootstrappingPathGenerator::remove_redundant_bootstrapping_paths()
{
    auto count = 0;
    for (auto i = 0; i < bootstrapping_paths.size(); i++)
    {
        for (auto j = i + 1;
             j < bootstrapping_paths.size() &&
             bootstrapping_paths[i].front() == bootstrapping_paths[j].front() &&
             bootstrapping_paths[i].back() == bootstrapping_paths[j].back();
             j++)
        {
            if (bootstrapping_paths[i].size() != bootstrapping_paths[j].size())
            {
                if (paths_are_redundant(bootstrapping_paths[i], bootstrapping_paths[j]))
                {
                    bootstrapping_paths.erase(bootstrapping_paths.begin() + j);
                    count++;
                    j--;
                }
            }
        }
    }
    std::cout << "Number of redundant bootstrapping paths removed: " << count << std::endl;
}

bool BootstrappingPathGenerator::paths_are_redundant(std::vector<OperationPtr> path1, std::vector<OperationPtr> path2)
{
    auto j = 0;
    for (auto i = 0; i < path1.size(); i++)
    {
        while (path1[i] != path2[j])
        {
            j++;
            if (j >= path2.size())
            {
                return false;
            }
        }
    }

    for (auto operation : path1)
    {
        std::cout << operation->id << ",";
    }
    std::cout << std::endl;

    for (auto operation : path2)
    {
        std::cout << operation->id << ",";
    }
    std::cout << std::endl;

    return true;
}

void BootstrappingPathGenerator::print_number_of_paths()
{
    std::unordered_map<OperationPtr, int> num_paths_to;
    for (auto operation : operations)
    {
        num_paths_to[operation] = 1;
        for (auto parent : operation->parent_ptrs)
        {
            num_paths_to[operation] += num_paths_to[parent] + 1;
        }
    }

    int result = std::accumulate(std::begin(num_paths_to), std::end(num_paths_to), 0,
                                 [](const int previous, const std::pair<const OperationPtr, int> &p)
                                 { return previous + p.second; });

    std::cout << result << std::endl;
}

void BootstrappingPathGenerator::print_bootstrapping_paths()
{
    for (auto path : bootstrapping_paths)
    {
        for (auto operation : path)
        {
            std::cout << operation->id << ",";
        }
        std::cout << std::endl;
    }
}

void BootstrappingPathGenerator::write_paths_to_file(std::ofstream &output_file)
{
    for (auto path : bootstrapping_paths)
    {
        for (auto operation : path)
        {
            output_file << operation->id << ",";
        }
        output_file << std::endl;
    }
}

std::vector<OperationList> BootstrappingPathGenerator::read_bootstrapping_paths(std::ifstream &input_file, OperationList operations)
{
    std::vector<OperationList> bootstrapping_paths;
    std::string line;

    while (std::getline(input_file, line))
    {
        auto line_as_list = split_string_by_character(line, ',');

        if (line_as_list[0] == "")
        {
            continue;
        }

        auto path_num = bootstrapping_paths.size();

        bootstrapping_paths.emplace_back();

        for (auto op_str : line_as_list)
        {
            auto op_num = std::stoi(op_str);
            auto op_ptr = get_operation_ptr_from_id(operations, op_num);
            op_ptr->path_nums.push_back(path_num);
            bootstrapping_paths.back().push_back(op_ptr);
        }
    }
    return bootstrapping_paths;
}

void BootstrappingPathGenerator::add_path_num_info_to_all_operations()
{
    auto path_num = 0;
    for (auto &path : bootstrapping_paths)
    {
        for (auto &operation : path)
        {
            operation->path_nums.push_back(path_num);
        }
        path_num++;
    }
}