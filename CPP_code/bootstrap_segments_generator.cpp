#include "bootstrap_segments_generator.h"

BootstrapSegmentGenerator::BootstrapSegmentGenerator(int argc, char **argv)
{
    parse_args(argc, argv);
    print_options();

    Program::ConstructorInput in;
    in.dag_filename = options.dag_filename;
    program = Program(in);
}

bool BootstrapSegmentGenerator::is_in_forced_generation_mode() const
{
    return options.force_generation;
}

bool BootstrapSegmentGenerator::segments_files_are_current() const
{
    struct stat executable_file_info;
    struct stat dag_file_info;
    struct stat standard_file_info;
    struct stat selective_file_info;

    stat(options.executable_filename.c_str(), &executable_file_info);
    stat(options.dag_filename.c_str(), &dag_file_info);
    auto result1 = stat((options.output_filename + "_standard.dat").c_str(), &standard_file_info);
    auto result2 = stat((options.output_filename + "_selective.dat").c_str(), &selective_file_info);

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

    options.executable_filename = argv[0];
    options.dag_filename = argv[1];
    options.output_filename = argv[2];
    options.num_levels = std::stoi(argv[3]);

    std::string options_string;
    for (int i = 4; i < argc; i++)
    {
        options_string += std::string(argv[i]) + " ";
    }

    options.write_text_files = !utl::arg_exists(options_string, "-n", "--no-text-files");

    auto initial_levels_string = utl::get_arg(options_string, "-i", "--initial_levels", help_info);
    if (!initial_levels_string.empty())
    {
        options.initial_levels = std::stoi(initial_levels_string);
    }

    options.force_generation = utl::arg_exists(options_string, "-F", "--force");
}

void BootstrapSegmentGenerator::print_options() const
{
    std::cout << "Generator using the following options." << std::endl;
    std::cout << "dag_filename: " << options.dag_filename << std::endl;
    std::cout << "output_filename: " << options.output_filename << std::endl;
    std::cout << "num_levels: " << options.num_levels << std::endl;
    std::cout << "write_text_files: " << (options.write_text_files ? "yes" : "no") << std::endl;
    std::cout << "initial_levels: " << options.initial_levels << std::endl;
    std::cout << "force_generation: " << (options.force_generation ? "yes" : "no") << std::endl;
}

void BootstrapSegmentGenerator::generate_bootstrap_segments()
{
    std::function<void()> find_ignorable_func = [this]()
    { find_operations_to_ignore(); };

    utl::perform_func_and_print_execution_time(find_ignorable_func, "Finding ignorable operations");

    std::function<void()> create_segs_func = [this]()
    { create_raw_bootstrap_segments(); };

    utl::perform_func_and_print_execution_time(create_segs_func, "Creating segments");

    if (bootstrap_segments.size() > 0)
    {
        sort_segments_and_report_time();

        remove_redundant_segments_and_report_time();
    }
    else
    {
        std::cout << "This program has no bootstrap segments." << std::endl;
        exit(0);
    }
}

void BootstrapSegmentGenerator::find_operations_to_ignore()
{
    auto &tffc = too_far_from_fresh_ciphertext;
    for (const auto op : program)
    {
        tffc[{op, -1}] = true;
    }

    for (int i = 0; i <= options.initial_levels; i++)
    {
        for (const auto &op : program)
        {
            int remaining_levels = i - (op->type == OperationType::MUL ? 1 : 0);

            if (op->has_no_parent_operations())
            {
                tffc[{op, i}] = (remaining_levels < 0);
            }
            else
            {
                bool too_far = false;
                for (const auto &p : op->parent_ptrs)
                {
                    too_far = too_far || tffc[{p, remaining_levels}];
                }
                tffc[{op, i}] = too_far;
            }
        }
    }
}

bool BootstrapSegmentGenerator::is_ignorable(const OperationPtr &operation) const
{
    return !too_far_from_fresh_ciphertext.at({operation, options.initial_levels});
}

void BootstrapSegmentGenerator::create_raw_bootstrap_segments()
{
    std::ranges::reverse_view reverse_program{program};
    for (const auto &op : reverse_program)
    {
        if (op->has_multiplication_child())
        {
            auto seg = BootstrapSegment();
            seg.add(op);
            back_segs[{op, 1}].push_back(seg);
        }
        else
        {
            get_segs_from_children(op, 1);
        }
    }

    for (int i = 2; i <= options.num_levels; i++)
    {
        for (const auto &op : reverse_program)
        {
            get_segs_from_children(op, i);
        }
    }

    for (const auto &op : program)
    {
        if (op->type == OperationType::MUL && !is_ignorable(op))
        {
            auto &op_segs = back_segs[{op, options.num_levels}];
            for (auto &seg : op_segs)
            {
                std::reverse(seg.begin(), seg.end());
            }
            bootstrap_segments.insert(bootstrap_segments.end(), op_segs.begin(), op_segs.end());
        }
    }
}

void BootstrapSegmentGenerator::get_segs_from_children(const OperationPtr &op, const int i)
{
    auto &op_segs = back_segs[{op, i}];
    // int remaining_levels = i - (op->type == OperationType::MUL ? 1 : 0);
    for (const auto &child : op->child_ptrs)
    {
        int remaining_levels = i - (child->type == OperationType::MUL ? 1 : 0);
        const auto &child_segs = back_segs[{child, remaining_levels}];
        op_segs.insert(op_segs.end(), child_segs.begin(), child_segs.end());
    }

    for (auto &seg : op_segs)
    {
        seg.add(op);
    }
}

void BootstrapSegmentGenerator::sort_segments()
{
    class comparator_class
    {
    public:
        bool operator()(BootstrapSegment segment1, BootstrapSegment segment2)
        {
            if (segment1.last_operation() == segment2.last_operation())
            {
                return segment1.size() < segment2.size();
            }
            else
            {
                return segment1.last_operation()->id < segment2.last_operation()->id;
            }
        }
    };

    auto starting_point_offset = 0;
    auto current_starting_id_to_sort = bootstrap_segments.front().first_operation();
    for (size_t i = 1; i < bootstrap_segments.size(); i++)
    {
        if (bootstrap_segments[i].first_operation() != current_starting_id_to_sort)
        {
            std::sort(bootstrap_segments.begin() + starting_point_offset, bootstrap_segments.begin() + i, comparator_class());
            starting_point_offset = i;
            current_starting_id_to_sort = bootstrap_segments[i].first_operation();
        }
    }

    std::sort(bootstrap_segments.begin() + starting_point_offset, bootstrap_segments.end(), comparator_class());
}

void BootstrapSegmentGenerator::sort_segments_and_report_time()
{
    std::function<void()> sort_func = [this]()
    { sort_segments(); };

    utl::perform_func_and_print_execution_time(sort_func, "Sorting segments");
}

void BootstrapSegmentGenerator::remove_redundant_segments()
{
    auto redundant_count = 0;
    for (size_t i = 0; i < bootstrap_segments.size(); i++)
    {
        auto j = i + 1;
        while (j < bootstrap_segments.size() &&
               bootstrap_segments[i].first_operation() == bootstrap_segments[j].first_operation() &&
               bootstrap_segments[i].last_operation() == bootstrap_segments[j].last_operation())
        {
            if (bootstrap_segments[i].size() < bootstrap_segments[j].size() &&
                segments_are_redundant(bootstrap_segments[i], bootstrap_segments[j]))
            {
                bootstrap_segments.erase(bootstrap_segments.begin() + j);
                redundant_count++;
            }
            else
            {
                j++;
            }
        }
    }
    std::cout << "Number of redundant bootstrap segments removed: " << redundant_count << std::endl;
}

void BootstrapSegmentGenerator::remove_redundant_segments_and_report_time()
{
    std::function<void()> redundant_func = [this]()
    { remove_redundant_segments(); };

    utl::perform_func_and_print_execution_time(redundant_func, "Removing redundant segments");
}

bool BootstrapSegmentGenerator::segments_are_redundant(const BootstrapSegment &smaller_segment, const BootstrapSegment &larger_segment) const
{
    auto j = 0;
    auto size_diff = larger_segment.size() - smaller_segment.size();
    for (size_t i = 0; i < smaller_segment.size(); i++)
    {
        while (smaller_segment.operation_at(i) != larger_segment.operation_at(j))
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

void BootstrapSegmentGenerator::print_number_of_segments() const
{
    std::unordered_map<OperationPtr, int> num_segments_to;
    for (auto operation : program)
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

void BootstrapSegmentGenerator::print_bootstrap_segments() const
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

void BootstrapSegmentGenerator::write_segments_to_files()
{
    program.set_bootstrap_segments(bootstrap_segments);

    write_files("standard");

    program.convert_segments_to_selective();

    write_files("selective");
}

void BootstrapSegmentGenerator::write_files(const std::string &suffix)
{
    const std::string filename_without_extension = options.output_filename + "_" + suffix;
    auto file_writer = FileWriter(std::ref(program));
    std::function<void()> write_binary_func = [file_writer, filename_without_extension]()
    { file_writer.write_segments_to_file(filename_without_extension + ".dat"); };

    utl::perform_func_and_print_execution_time(write_binary_func, "Writing " + suffix + " file");

    if (options.write_text_files)
    {
        std::function<void()> write_text_func = [file_writer, filename_without_extension]()
        { file_writer.write_segments_to_text_file(filename_without_extension + ".txt"); };

        utl::perform_func_and_print_execution_time(write_text_func, "Writing " + suffix + " text file");
    }
}

void BootstrapSegmentGenerator::remove_last_operation_from_segments()
{
    for (auto &segment : bootstrap_segments)
    {
        segment.remove_last_operation();
    }
}

int main(int argc, char **argv)
{
    auto generator = BootstrapSegmentGenerator(argc, argv);

    if (generator.is_in_forced_generation_mode() || !generator.segments_files_are_current())
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