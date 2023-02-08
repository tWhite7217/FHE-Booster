#include "bootstrapping_path_generator.h"

BootstrappingPathGenerator::BootstrappingPathGenerator(int argc, char **argv)
{
    parse_args(argc, argv);
    print_options();

    standard_output_file_path = options.output_file_path + ".txt";
    selective_output_file_path = options.output_file_path + "_selective.txt";
}

bool BootstrappingPathGenerator::is_in_forced_generation_mode()
{
    return options.force_generation;
}

bool BootstrappingPathGenerator::bootstrapping_files_are_current()
{
    struct stat executable_file_info;
    struct stat dag_file_info;
    struct stat standard_file_info;
    struct stat selective_file_info;

    stat(executable_file.c_str(), &executable_file_info);
    stat(options.dag_file_path.c_str(), &dag_file_info);
    auto result1 = stat(standard_output_file_path.c_str(), &standard_file_info);
    auto result2 = stat(selective_output_file_path.c_str(), &selective_file_info);

    auto executable_time = executable_file_info.st_mtime;
    auto dag_time = dag_file_info.st_mtime;
    time_t standard_time;
    time_t selective_time;
    if (result1 == 0 && result2 == 0)
    {
        standard_time = standard_file_info.st_mtime;
        selective_time = selective_file_info.st_mtime;
    }
    else
    {
        return false;
    }

    return standard_time > executable_time && standard_time > dag_time &&
           selective_time > executable_time && selective_time > dag_time;
}

void BootstrappingPathGenerator::parse_args(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << help_info << std::endl;
        exit(1);
    }

    executable_file = argv[0];
    options.dag_file_path = argv[1];
    options.output_file_path = argv[2];
    options.num_levels = std::stoi(argv[3]);

    std::string options_string;
    for (auto i = 4; i < argc; i++)
    {
        options_string += std::string(argv[i]) + " ";
    }

    auto initial_levels_string = get_arg(options_string, "-i", "--initial_levels", help_info);
    if (!initial_levels_string.empty())
    {
        options.initial_levels = std::stoi(initial_levels_string);
    }

    options.force_generation = arg_exists(options_string, "-F", "--force");
}

void BootstrappingPathGenerator::print_options()
{
    std::cout << "Generator using the following options." << std::endl;
    std::cout << "dag_file_path: " << options.dag_file_path << std::endl;
    std::cout << "output_file_path: " << options.output_file_path << std::endl;
    std::cout << "num_levels: " << options.num_levels << std::endl;
    std::cout << "initial_levels: " << options.initial_levels << std::endl;
    std::cout << "force_generation: " << (options.force_generation ? "yes" : "no") << std::endl;
}

void BootstrappingPathGenerator::generate_bootstrapping_paths()
{
    create_raw_bootstrapping_paths();
    remove_redundant_bootstrapping_paths();
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

void BootstrappingPathGenerator::remove_redundant_bootstrapping_paths()
{
    auto duplicate_count = 0;
    auto redundant_count = 0;
    for (auto i = 0; i < bootstrapping_paths.size(); i++)
    {
        auto j = i + 1;
        while (j < bootstrapping_paths.size() &&
               bootstrapping_paths[i].front() == bootstrapping_paths[j].front() &&
               bootstrapping_paths[i].back() == bootstrapping_paths[j].back())
        {
            if (bootstrapping_paths[i].size() <= bootstrapping_paths[j].size())
            {
                if (paths_are_redundant(bootstrapping_paths[i], bootstrapping_paths[j]))
                {
                    bootstrapping_paths.erase(bootstrapping_paths.begin() + j);
                    if (bootstrapping_paths[i].size() == bootstrapping_paths[j].size())
                    {
                        duplicate_count++;
                    }
                    else
                    {
                        redundant_count++;
                    }
                }
                else
                {
                    j++;
                }
            }
        }
    }
    std::cout << "Number of duplicate bootstrapping paths removed: " << duplicate_count << std::endl;
    std::cout << "Number of redundant bootstrapping paths removed: " << redundant_count << std::endl;
}

bool BootstrappingPathGenerator::paths_are_redundant(std::vector<OperationPtr> smaller_path, std::vector<OperationPtr> larger_path)
{
    auto j = 0;
    auto size_diff = larger_path.size() - smaller_path.size();
    for (auto i = 0; i < smaller_path.size(); i++)
    {
        while (smaller_path[i] != larger_path[j])
        {
            j++;
            if (j - i > size_diff)
            {
                return false;
            }
        }
    }
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

void BootstrappingPathGenerator::write_segments_to_file(std::ofstream &output_file)
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

void BootstrappingPathGenerator::write_segments_to_files()
{
    std::ofstream selective_file(selective_output_file_path);
    write_segments_to_file(selective_file);
    selective_file.close();

    convert_segments_to_standard();

    std::ofstream standard_file(standard_output_file_path);
    write_segments_to_file(standard_file);
    standard_file.close();
}

void BootstrappingPathGenerator::convert_segments_to_standard()
{
    remove_last_operation_from_bootstrapping_paths();
    remove_redundant_bootstrapping_paths();
}

void BootstrappingPathGenerator::remove_last_operation_from_bootstrapping_paths()
{
    for (auto &path : bootstrapping_paths)
    {
        path.pop_back();
    }
}

int main(int argc, char **argv)
{
    auto generator = BootstrappingPathGenerator(argc, argv);

    if (generator.is_in_forced_generation_mode() || !generator.bootstrapping_files_are_current())
    {
        generator.generate_bootstrapping_paths();
        generator.write_segments_to_files();
    }
    else
    {
        std::cout << "Segments generation cancelled. Current bootstrapping_segments files" << std::endl;
        std::cout << "seem up to date. Use argument -F/--force to generate segments anyway." << std::endl;
        return 0;
    }
    return 0;
}