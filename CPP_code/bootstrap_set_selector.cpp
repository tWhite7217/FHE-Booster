#include "bootstrap_set_selector.h"

BootstrapSetSelector::BootstrapSetSelector(int argc, char **argv)
{
    parse_args(argc, argv);

    Program::ConstructorInput in;
    in.dag_filename = options.dag_filename;
    in.segments_filename = options.segments_filename;

    program = Program(in);
}

void BootstrapSetSelector::choose_and_output_bootstrap_sets()
{
    while (set_index < num_sets)
    {
        program.reset_bootstrap_set();
        print_options();
        choose_operations_to_bootstrap();
        program.write_bootstrapping_set_to_file(options.output_filenames[set_index] + ".lgr");
        set_index++;
    }
}

void BootstrapSetSelector::choose_operations_to_bootstrap()
{
    auto count = 0;
    while (!program.bootstrap_segments_are_satisfied())
    {
        program.update_num_segments_for_every_operation();
        max_num_segments = program.get_maximum_num_segments();
        if (options.slack_weight[set_index] != 0)
        {
            program.update_ESTs_and_LSTs();
            max_slack = program.get_maximum_slack();
        }
        if (options.urgency_weight[set_index] != 0)
        {
            program.update_all_bootstrap_urgencies();
        }
        choose_operation_to_bootstrap_based_on_score();
    }
}

void BootstrapSetSelector::choose_operation_to_bootstrap_based_on_score()
{
    auto max_score = -1;
    OperationPtr max_score_operation;
    for (const auto &operation : program)
    {
        if (!operation->is_bootstrapped())
        {
            auto score = get_score(operation);
            if (score > max_score)
            {
                max_score = score;
                max_score_operation = operation;
            }
        }
    }

    for (const auto &child : max_score_operation->child_ptrs)
    {
        max_score_operation->bootstrap_children.insert(child);
    }
}

double BootstrapSetSelector::get_score(const OperationPtr &operation)
{
    auto num_segments = operation->num_unsatisfied_segments;
    if (num_segments == 0)
    {
        return 0;
    }

    double normalized_num_segments = ((double)num_segments) / max_num_segments;
    double normalized_slack = ((double)operation->get_slack()) / max_slack;

    return std::max(options.segments_weight[set_index] * normalized_num_segments +
                        options.slack_weight[set_index] * normalized_slack +
                        options.urgency_weight[set_index] * operation->bootstrap_urgency,
                    0.0);
}

void BootstrapSetSelector::parse_args(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << help_info << std::endl;
        exit(1);
    }

    options.dag_filename = argv[1];
    options.segments_filename = argv[2];
    options.output_filenames = split_string_by_character(argv[3], ',');
    num_sets = options.output_filenames.size();

    options.num_levels = std::stoi(argv[4]);

    const std::function<int(std::string)> stoi_function = [](std::string str)
    { return std::stoi(str); };

    std::string options_string;
    for (auto i = 5; i < argc; i++)
    {
        options_string += std::string(argv[i]) + " ";
    }

    options.segments_weight = get_list_arg(options_string, "-s", "--segments-weight", help_info, num_sets, 0, stoi_function);
    options.slack_weight = get_list_arg(options_string, "-r", "--slack-weight", help_info, num_sets, 0, stoi_function);
    options.urgency_weight = get_list_arg(options_string, "-u", "--urgency-weight", help_info, num_sets, 0, stoi_function);
}

void BootstrapSetSelector::print_options()
{
    std::cout << "dag_filename: " << options.dag_filename << std::endl;
    std::cout << "segments_filename: " << options.segments_filename << std::endl;
    std::cout << "output_filename: " << options.output_filenames[set_index] << std::endl;
    std::cout << "num_levels: " << options.num_levels << std::endl;
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