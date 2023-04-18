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
    const int minimum_arguments = 4;

    if (argc < minimum_arguments)
    {
        std::cout << help_info << std::endl;
        exit(1);
    }

    options.executable_filename = argv[0];
    options.dag_filename = argv[1];
    options.output_filename = argv[2];
    options.num_levels = std::stoi(argv[3]);

    std::string options_string = utl::make_options_string(argc, argv, minimum_arguments);

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
    if (options.initial_levels > 0)
    {
        std::function<void()> find_ignorable_func = [this]()
        { find_operations_to_ignore(); };

        utl::perform_func_and_print_execution_time(find_ignorable_func, "Finding ignorable operations");
    }
    else
    {
        auto &tffc = too_far_from_fresh_ciphertext;
        for (const auto op : program)
        {
            tffc[{op, options.initial_levels + 1}] = true;
        }
    }

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

    for (int i = 0; i <= options.initial_levels + 1; i++)
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
    return !too_far_from_fresh_ciphertext.at({operation, options.initial_levels + 1});
}

void BootstrapSegmentGenerator::create_raw_bootstrap_segments()
{
    std::ranges::reverse_view reverse_program{program};
    for (const auto &op : reverse_program)
    {
        if (!is_ignorable(op))
        {
            get_segs_from_children(op, 1);
            if (op->has_multiplication_child())
            {
                auto seg = BootstrapSegment();
                seg.add(op);
                back_segs[1][op].push_back(seg);
                for (auto &seg : back_segs[1][op])
                {
                    seg.set_last_mul(op);
                }
            }
        }
    }

    for (int i = 2; i <= options.num_levels; i++)
    {
        for (const auto &op : reverse_program)
        {
            if (!is_ignorable(op))
            {
                get_segs_from_children(op, i);
                clean_children_memory(op, i);
            }
        }
        back_segs.erase(i - 1);
    }

    for (const auto &op : program)
    {
        if (op->type == OperationType::MUL && !is_ignorable(op))
        {
            auto &op_segs = back_segs[options.num_levels][op];
            for (auto &seg : op_segs)
            {
                std::reverse(seg.begin(), seg.end());
            }
            bootstrap_segments.insert(bootstrap_segments.end(), std::make_move_iterator(op_segs.begin()), std::make_move_iterator(op_segs.end()));
        }
    }

    back_segs.clear();
}

void BootstrapSegmentGenerator::clean_children_memory(const OperationPtr op, const int i)
{
    const bool last_level = (i == options.num_levels);
    for (const auto &child : op->child_ptrs)
    {
        auto deleted = clean_child_memory(child, i, op->id);
        if (last_level && deleted && (child->type != OperationType::MUL || is_ignorable(child)))
        {
            back_segs[i].erase(child);
        }
    }
}

bool BootstrapSegmentGenerator::clean_child_memory(const OperationPtr child, const int i, const int current_id)
{
    for (const auto &parent : child->parent_ptrs)
    {
        if (parent->id < current_id)
        {
            return false;
        }
    }

    back_segs[i - 1].erase(child);
    return true;
}

void BootstrapSegmentGenerator::get_segs_from_children(const OperationPtr &op, const int i)
{
    auto &op_segs = back_segs[i][op];
    for (const auto &child : op->child_ptrs)
    {
        int remaining_levels = i - (child->type == OperationType::MUL ? 1 : 0);
        const auto &child_segs = back_segs[remaining_levels][child];
        op_segs.insert(op_segs.end(), child_segs.begin(), child_segs.end());
    }

    for (auto &seg : op_segs)
    {
        seg.add(op);
        seg.shrink_to_fit();
    }
}

void BootstrapSegmentGenerator::sort_segments()
{
    class comparator_class
    {
    public:
        bool operator()(BootstrapSegment segment1, BootstrapSegment segment2)
        {
            if (segment1.last_multiplication() == segment2.last_multiplication())
            {
                return segment1.size() < segment2.size();
            }
            else
            {
                return segment1.last_multiplication()->id < segment2.last_multiplication()->id;
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
    for (size_t i = 0; i < bootstrap_segments.size(); i++)
    {
        auto j = i + 1;
        while (j < bootstrap_segments.size() &&
               bootstrap_segments[i].first_operation() == bootstrap_segments[j].first_operation() &&
               bootstrap_segments[i].last_multiplication() == bootstrap_segments[j].last_multiplication())
        {
            if (bootstrap_segments[i].size() < bootstrap_segments[j].size() &&
                segments_are_redundant(bootstrap_segments[i], bootstrap_segments[j]))
            {
                // std::cout << "small" << std::endl;
                // bootstrap_segments[i].print();
                // std::cout << "large" << std::endl;
                // bootstrap_segments[j].print();
                removed_segments.push_back(*(bootstrap_segments.begin() + j));
                bootstrap_segments.erase(bootstrap_segments.begin() + j);
            }
            else
            {
                j++;
            }
        }
    }
    std::cout << "Number of redundant bootstrap segments removed: " << removed_segments.size() << std::endl;
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
    write_files("standard");

    reinstate_removed_redundant_segments();
    convert_segments_to_selective();

    write_files("selective");
}

void BootstrapSegmentGenerator::convert_segments_to_selective()
{
    std::vector<BootstrapSegment> new_segments;
    for (const auto &segment : bootstrap_segments)
    {
        for (const auto &child : segment.last_operation()->child_ptrs)
        {
            new_segments.push_back(segment);
            new_segments.back().add(child);
            new_segments.back().shrink_to_fit();
        }
    }
    new_segments.shrink_to_fit();
    bootstrap_segments = new_segments;
}

void BootstrapSegmentGenerator::write_files(const std::string &suffix)
{
    const std::string filename_without_extension = options.output_filename + "_" + suffix;
    std::function<void()> write_binary_func = [this, &filename_without_extension]()
    { FileWriter::write_segments_to_file(bootstrap_segments, filename_without_extension + ".dat"); };

    utl::perform_func_and_print_execution_time(write_binary_func, "Writing " + suffix + " file");

    if (options.write_text_files)
    {
        std::function<void()> write_text_func = [this, &filename_without_extension]()
        { FileWriter::write_segments_to_text_file(bootstrap_segments, filename_without_extension + ".txt"); };

        utl::perform_func_and_print_execution_time(write_text_func, "Writing " + suffix + " text file");
    }
}

void BootstrapSegmentGenerator::reinstate_removed_redundant_segments()
{
    bootstrap_segments.insert(bootstrap_segments.end(), std::make_move_iterator(removed_segments.begin()), std::make_move_iterator(removed_segments.end()));
    removed_segments.clear();
    removed_segments.shrink_to_fit();
}

std::string BootstrapSegmentGenerator::get_log_filename() const
{
    return options.output_filename + ".log";
}

int main(int argc, char **argv)
{

    auto generator = BootstrapSegmentGenerator(argc, argv);

    if (generator.is_in_forced_generation_mode() || !generator.segments_files_are_current())
    {
        std::ofstream log_file(generator.get_log_filename());

        std::function<void()> main_func = [&generator]()
        {
            generator.generate_bootstrap_segments();
            generator.write_segments_to_files();
        };

        utl::perform_func_and_print_execution_time(main_func, log_file);
    }
    else
    {
        std::cout << "Segments generation cancelled. Current bootstrap_segments files" << std::endl;
        std::cout << "seem up to date. Use argument -F/--force to generate segments anyway." << std::endl;
        return 0;
    }
    return 0;
}