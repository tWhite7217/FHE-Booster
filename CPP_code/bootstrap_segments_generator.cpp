#include "bootstrap_segments_generator.h"

BootstrapSegmentGenerator::BootstrapSegmentGenerator(int argc, char **argv)
{
    parse_args(argc, argv);
    print_options();

    standard_output_filename = options.output_filename + ".txt";
    selective_output_filename = options.output_filename + "_selective.txt";

    InputParser parser;
    parser.parse_input_to_generate_operations(options.dag_filename);
    operations = parser.get_operations();
}

bool BootstrapSegmentGenerator::is_in_forced_generation_mode()
{
    return options.force_generation;
}

bool BootstrapSegmentGenerator::bootstrap_files_are_current()
{
    struct stat executable_file_info;
    struct stat dag_file_info;
    struct stat standard_file_info;
    struct stat selective_file_info;

    stat(executable_file.c_str(), &executable_file_info);
    stat(options.dag_filename.c_str(), &dag_file_info);
    auto result1 = stat(standard_output_filename.c_str(), &standard_file_info);
    auto result2 = stat(selective_output_filename.c_str(), &selective_file_info);

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

void BootstrapSegmentGenerator::parse_args(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << help_info << std::endl;
        exit(1);
    }

    executable_filename = argv[0];
    options.dag_filename = argv[1];
    options.output_filename = argv[2];
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

void BootstrapSegmentGenerator::print_options()
{
    std::cout << "Generator using the following options." << std::endl;
    std::cout << "dag_filename: " << options.dag_filename << std::endl;
    std::cout << "output_filename: " << options.output_filename << std::endl;
    std::cout << "num_levels: " << options.num_levels << std::endl;
    std::cout << "initial_levels: " << options.initial_levels << std::endl;
    std::cout << "force_generation: " << (options.force_generation ? "yes" : "no") << std::endl;
}

void BootstrapSegmentGenerator::generate_bootstrap_segments()
{
    create_raw_bootstrap_segments();
    remove_redundant_bootstrap_segments();
}

void BootstrapSegmentGenerator::create_raw_bootstrap_segments()
{
    for (auto operation : operations)
    {
        std::vector<std::vector<OperationPtr>> bootstrap_segments_to_add;
        if (operation->type == "MUL")
        {
            bootstrap_segments_to_add = create_bootstrap_segments_helper(operation, {}, 0);
        }
        bootstrap_segments.insert(bootstrap_segments.end(), bootstrap_segments_to_add.begin(), bootstrap_segments_to_add.end());
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
    if (bootstrap_segments.size() > 0)
    {
        auto current_starting_id_to_sort = bootstrap_segments[0][0];
        for (auto i = 1; i < bootstrap_segments.size(); i++)
        {
            if (bootstrap_segments[i][0] != current_starting_id_to_sort)
            {
                std::sort(bootstrap_segments.begin() + starting_point_offset, bootstrap_segments.begin() + i, comparator_class());
                starting_point_offset = i;
                current_starting_id_to_sort = bootstrap_segments[i][0];
            }
        }
    }
    else
    {
        std::cout << "This program has no bootstrap segments." << std::endl;
    }
}

std::vector<std::vector<OperationPtr>> BootstrapSegmentGenerator::create_bootstrap_segments_helper(OperationPtr operation, std::vector<OperationPtr> segment, int num_multiplications)
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
        segments_to_add = create_bootstrap_segments_helper(child, segment, num_multiplications);
        segments_to_return.insert(segments_to_return.end(), segments_to_add.begin(), segments_to_add.end());
    }
    return segments_to_return;
}

void BootstrapSegmentGenerator::remove_redundant_bootstrap_segments()
{
    auto duplicate_count = 0;
    auto redundant_count = 0;
    for (auto i = 0; i < bootstrap_segments.size(); i++)
    {
        auto j = i + 1;
        while (j < bootstrap_segments.size() &&
               bootstrap_segments[i].front() == bootstrap_segments[j].front() &&
               bootstrap_segments[i].back() == bootstrap_segments[j].back())
        {
            if (bootstrap_segments[i].size() <= bootstrap_segments[j].size())
            {
                if (segments_are_redundant(bootstrap_segments[i], bootstrap_segments[j]))
                {
                    bootstrap_segments.erase(bootstrap_segments.begin() + j);
                    if (bootstrap_segments[i].size() == bootstrap_segments[j].size())
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
    std::cout << "Number of duplicate bootstrap segments removed: " << duplicate_count << std::endl;
    std::cout << "Number of redundant bootstrap segments removed: " << redundant_count << std::endl;
}

bool BootstrapSegmentGenerator::segments_are_redundant(std::vector<OperationPtr> smaller_segment, std::vector<OperationPtr> larger_segment)
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

void BootstrapSegmentGenerator::print_number_of_segments()
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

void BootstrapSegmentGenerator::print_bootstrap_segments()
{
    for (auto segment : bootstrap_segments)
    {
        for (auto operation : segment)
        {
            std::cout << operation->id << ",";
        }
        std::cout << std::endl;
    }
}

void BootstrapSegmentGenerator::write_segments_to_file(std::ofstream &output_file)
{
    for (auto segment : bootstrap_segments)
    {
        for (auto operation : segment)
        {
            output_file << operation->id << ",";
        }
        output_file << std::endl;
    }
}

void BootstrapSegmentGenerator::write_segments_to_files()
{
    std::ofstream selective_file(selective_output_filename);
    write_segments_to_file(selective_file);
    selective_file.close();

    convert_segments_to_standard();

    std::ofstream standard_file(standard_output_filename);
    write_segments_to_file(standard_file);
    standard_file.close();
}

void BootstrapSegmentGenerator::convert_segments_to_standard()
{
    remove_last_operation_from_bootstrap_segments();
    remove_redundant_bootstrap_segments();
}

void BootstrapSegmentGenerator::remove_last_operation_from_bootstrap_segments()
{
    for (auto &segment : bootstrap_segments)
    {
        segment.pop_back();
    }
}

int main(int argc, char **argv)
{
    auto generator = BootstrapSegmentGenerator(argc, argv);

    if (generator.is_in_forced_generation_mode() || !generator.bootstrap_files_are_current())
    {
        generator.generate_bootstrap_segments();
        generator.write_segments_to_files();
    }
    else
    {
        std::cout << "Segments generation cancelled. Current bootstrap_segments files" << std::endl;
        std::cout << "seem up to date. Use argument -F/--force to generate segments anyway." << std::endl;
        return 0;
    }
    return 0;
}