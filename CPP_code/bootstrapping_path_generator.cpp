#include "bootstrapping_segment_generator.h"

BootstrappingSegmentGenerator::BootstrappingSegmentGenerator(int argc, char **argv)
{
    parse_args(argc, argv);
    print_options();

    standard_output_file_path = options.output_file_path + ".txt";
    selective_output_file_path = options.output_file_path + "_selective.txt";
}

bool BootstrappingSegmentGenerator::is_in_forced_generation_mode()
{
    return options.force_generation;
}

bool BootstrappingSegmentGenerator::bootstrapping_files_are_current()
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

void BootstrappingSegmentGenerator::parse_args(int argc, char **argv)
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

void BootstrappingSegmentGenerator::print_options()
{
    std::cout << "Generator using the following options." << std::endl;
    std::cout << "dag_file_path: " << options.dag_file_path << std::endl;
    std::cout << "output_file_path: " << options.output_file_path << std::endl;
    std::cout << "num_levels: " << options.num_levels << std::endl;
    std::cout << "initial_levels: " << options.initial_levels << std::endl;
    std::cout << "force_generation: " << (options.force_generation ? "yes" : "no") << std::endl;
}

void BootstrappingSegmentGenerator::generate_bootstrapping_segments()
{
    create_raw_bootstrapping_segments();
    remove_redundant_bootstrapping_segments();
}

void BootstrappingSegmentGenerator::create_raw_bootstrapping_segments()
{
    for (auto operation : operations)
    {
        std::vector<std::vector<OperationPtr>> bootstrapping_segments_to_add;
        if (operation->type == "MUL")
        {
            bootstrapping_segments_to_add = create_bootstrapping_segments_helper(operation, {}, 0);
        }
        bootstrapping_segments.insert(bootstrapping_segments.end(), bootstrapping_segments_to_add.begin(), bootstrapping_segments_to_add.end());
    }

    class comparator_class
    {
    public:
        bool operator()(std::vector<OperationPtr> segment1, std::vector<OperationPtr> segment2)
        {
            if (segment1.back() == segment2.back())
            {
                return segment1.size() < segment2.size();
            }
            else
            {
                return segment1.back()->id < segment2.back()->id;
            }
        }
    };

    auto starting_point_offset = 0;
    if (bootstrapping_segments.size() > 0)
    {
        auto current_starting_id_to_sort = bootstrapping_segments[0][0];
        for (auto i = 1; i < bootstrapping_segments.size(); i++)
        {
            if (bootstrapping_segments[i][0] != current_starting_id_to_sort)
            {
                std::sort(bootstrapping_segments.begin() + starting_point_offset, bootstrapping_segments.begin() + i, comparator_class());
                starting_point_offset = i;
                current_starting_id_to_sort = bootstrapping_segments[i][0];
            }
        }
    }
    else
    {
        std::cout << "This program has no bootstrapping segments." << std::endl;
    }
}

std::vector<std::vector<OperationPtr>> BootstrappingSegmentGenerator::create_bootstrapping_segments_helper(OperationPtr operation, std::vector<OperationPtr> segment, int num_multiplications)
{
    if (operation->type == "MUL")
    {
        num_multiplications++;
    }
    segment.push_back(operation);
    if (num_multiplications >= gained_levels && operation_has_multiplication_child(operation))
    {
        std::vector<OperationList> segments_to_return;
        for (auto child : operation->child_ptrs)
        {
            auto segment_to_return = segment;
            segment_to_return.push_back(child);
            segments_to_return.push_back(segment_to_return);
        }
        return segments_to_return;
    }
    else if (operation->child_ptrs.size() == 0)
    {
        return {};
    }

    std::vector<OperationList> segments_to_return;
    for (auto child : operation->child_ptrs)
    {
        std::vector<std::vector<OperationPtr>> segments_to_add;
        segments_to_add = create_bootstrapping_segments_helper(child, segment, num_multiplications);
        segments_to_return.insert(segments_to_return.end(), segments_to_add.begin(), segments_to_add.end());
    }
    return segments_to_return;
}

void BootstrappingSegmentGenerator::remove_redundant_bootstrapping_segments()
{
    auto duplicate_count = 0;
    auto redundant_count = 0;
    for (auto i = 0; i < bootstrapping_segments.size(); i++)
    {
        auto j = i + 1;
        while (j < bootstrapping_segments.size() &&
               bootstrapping_segments[i].front() == bootstrapping_segments[j].front() &&
               bootstrapping_segments[i].back() == bootstrapping_segments[j].back())
        {
            if (bootstrapping_segments[i].size() <= bootstrapping_segments[j].size())
            {
                if (segments_are_redundant(bootstrapping_segments[i], bootstrapping_segments[j]))
                {
                    bootstrapping_segments.erase(bootstrapping_segments.begin() + j);
                    if (bootstrapping_segments[i].size() == bootstrapping_segments[j].size())
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
    std::cout << "Number of duplicate bootstrapping segments removed: " << duplicate_count << std::endl;
    std::cout << "Number of redundant bootstrapping segments removed: " << redundant_count << std::endl;
}

bool BootstrappingSegmentGenerator::segments_are_redundant(std::vector<OperationPtr> smaller_segment, std::vector<OperationPtr> larger_segment)
{
    auto j = 0;
    auto size_diff = larger_segment.size() - smaller_segment.size();
    for (auto i = 0; i < smaller_segment.size(); i++)
    {
        while (smaller_segment[i] != larger_segment[j])
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

void BootstrappingSegmentGenerator::print_number_of_segments()
{
    std::unordered_map<OperationPtr, int> num_segments_to;
    for (auto operation : operations)
    {
        num_segments_to[operation] = 1;
        for (auto parent : operation->parent_ptrs)
        {
            num_segments_to[operation] += num_segments_to[parent] + 1;
        }
    }

    int result = std::accumulate(std::begin(num_segments_to), std::end(num_segments_to), 0,
                                 [](const int previous, const std::pair<const OperationPtr, int> &p)
                                 { return previous + p.second; });

    std::cout << result << std::endl;
}

void BootstrappingSegmentGenerator::print_bootstrapping_segments()
{
    for (auto segment : bootstrapping_segments)
    {
        for (auto operation : segment)
        {
            std::cout << operation->id << ",";
        }
        std::cout << std::endl;
    }
}

void BootstrappingSegmentGenerator::write_segments_to_file(std::ofstream &output_file)
{
    for (auto segment : bootstrapping_segments)
    {
        for (auto operation : segment)
        {
            output_file << operation->id << ",";
        }
        output_file << std::endl;
    }
}

void BootstrappingSegmentGenerator::write_segments_to_files()
{
    std::ofstream selective_file(selective_output_file_path);
    write_segments_to_file(selective_file);
    selective_file.close();

    convert_segments_to_standard();

    std::ofstream standard_file(standard_output_file_path);
    write_segments_to_file(standard_file);
    standard_file.close();
}

void BootstrappingSegmentGenerator::convert_segments_to_standard()
{
    remove_last_operation_from_bootstrapping_segments();
    remove_redundant_bootstrapping_segments();
}

void BootstrappingSegmentGenerator::remove_last_operation_from_bootstrapping_segments()
{
    for (auto &segment : bootstrapping_segments)
    {
        segment.pop_back();
    }
}

int main(int argc, char **argv)
{
    auto generator = BootstrappingSegmentGenerator(argc, argv);

    if (generator.is_in_forced_generation_mode() || !generator.bootstrapping_files_are_current())
    {
        generator.generate_bootstrapping_segments();
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