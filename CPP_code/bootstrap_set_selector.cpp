#include "bootstrap_set_selector.h"

BootstrapSetSelector::BootstrapSetSelector(int argc, char **argv)
{
    parse_args(argc, argv);

    Program::ConstructorInput in;
    in.dag_filename = options.dag_filename;
    in.segments_filename = options.segments_filename;

    program = Program(in);

    program.set_boot_mode(BootstrapMode::COMPLETE);
}

void BootstrapSetSelector::choose_and_output_bootstrap_sets()
{
    std::function<void()> one_iteration_func = [this]()
    {
        program.reset_bootstrap_set();
        print_options();

        std::function<void()> choose_operations_func = [this]()
        { choose_operations_to_bootstrap(); };

        utl::perform_func_and_print_execution_time(choose_operations_func, "Choosing operations to bootstrap");

        std::function<void()> write_file_func = [this]()
        {
            auto file_writer = FileWriter(std::ref(program));
            file_writer.write_bootstrapping_set_to_file(options.output_filenames[set_index] + ".lgr");
        };

        utl::perform_func_and_print_execution_time(write_file_func, "Writing bootstrap set to file");
    };

    while (set_index < num_sets)
    {
        std::ofstream log_file(get_log_filename());

        utl::perform_func_and_print_execution_time(one_iteration_func, log_file);

        set_index++;
    }
}

void BootstrapSetSelector::choose_operations_to_bootstrap()
{
    auto count = 0;
    program.initialize_unsatisfied_segment_indexes();
    program.initialize_num_segments_for_every_operation();
    program.initialize_alive_segment_indexes();
    program.initialize_operation_to_segments_map();
    while (program.has_unsatisfied_bootstrap_segments())
    {
        max_num_segments = program.get_maximum_num_segments();
        if (options.slack_weight[set_index] != 0)
        {
            program.update_slack_for_every_operation();
            max_slack = program.get_maximum_slack();
        }
        if (options.urgency_weight[set_index] != 0)
        {
            program.update_all_bootstrap_urgencies();
        }
        auto chosen_op = choose_operation_to_bootstrap_based_on_score();

        auto newly_satisfied_segments = program.update_unsatisfied_segments_and_num_segments_for_every_operation();
        if (options.urgency_weight[set_index] != 0)
        {
            program.update_alive_segments(chosen_op, newly_satisfied_segments);
        }
    }
}

OperationPtr BootstrapSetSelector::choose_operation_to_bootstrap_based_on_score()
{
    auto max_score = -1;
    OperationPtr max_score_operation;
    // std::ranges::reverse_view reverse_program{program};
    // for (const auto &operation : reverse_program)
    for (const auto &operation : program)
    {
        if (!operation->is_bootstrapped())
        {
            auto score = get_score(operation);
            if (score > max_score && operation->num_unsatisfied_segments > 0)
            {
                max_score = score;
                max_score_operation = operation;
            }
        }
    }

    max_score_operation->bootstrap_children = max_score_operation->child_ptrs;
    return max_score_operation;
}

double BootstrapSetSelector::get_score(const OperationPtr &operation) const
{
    auto num_segments = operation->num_unsatisfied_segments;
    if (num_segments == 0)
    {
        return 0;
    }
    double normalized_num_segments = ((double)num_segments) / max_num_segments;

    double normalized_slack;
    if (max_slack == 0)
    {
        normalized_slack = 0;
    }
    else
    {
        normalized_slack = ((double)operation->get_slack()) / max_slack;
    }

    double score =
        options.segments_weight[set_index] * normalized_num_segments +
        options.slack_weight[set_index] * normalized_slack +
        options.urgency_weight[set_index] * operation->bootstrap_urgency;

    return std::max(score, 0.0);
}

void BootstrapSetSelector::parse_args(int argc, char **argv)
{
    const int minimum_arguments = 4;

    if (argc < minimum_arguments)
    {
        std::cout << help_info << std::endl;
        exit(1);
    }

    options.dag_filename = argv[1];
    options.segments_filename = argv[2];
    options.output_filenames = utl::split_string_by_character(argv[3], ',');
    num_sets = options.output_filenames.size();

    // options.num_levels = std::stoi(argv[4]);

    const std::function<int(std::string)> stoi_function = [](std::string str)
    { return std::stoi(str); };

    std::string options_string = utl::make_options_string(argc, argv, minimum_arguments);

    options.segments_weight = utl::get_list_arg(options_string, "-s", "--segments-weight", help_info, num_sets, 0, stoi_function);
    options.slack_weight = utl::get_list_arg(options_string, "-r", "--slack-weight", help_info, num_sets, 0, stoi_function);
    options.urgency_weight = utl::get_list_arg(options_string, "-u", "--urgency-weight", help_info, num_sets, 0, stoi_function);
}

std::string BootstrapSetSelector::get_log_filename() const
{
    return options.output_filenames[set_index] + ".lgr.log";
}

void BootstrapSetSelector::print_options() const
{
    std::cout << "dag_filename: " << options.dag_filename << std::endl;
    std::cout << "segments_filename: " << options.segments_filename << std::endl;
    std::cout << "output_filename: " << options.output_filenames[set_index] << std::endl;
    // std::cout << "num_levels: " << options.num_levels << std::endl;
    std::cout << "segments_weight: " << options.segments_weight[set_index] << std::endl;
    std::cout << "slack_weight: " << options.slack_weight[set_index] << std::endl;
    std::cout << "urgency_weight: " << options.urgency_weight[set_index] << std::endl;
}

int main(int argc, char **argv)
{
    BootstrapSetSelector bootstrap_set_selector = BootstrapSetSelector(argc, argv);

    bootstrap_set_selector.choose_and_output_bootstrap_sets();

    return 0;
}