#include "bootstrap_set_selector.h"

BootstrapSetSelector::BootstrapSetSelector(int argc, char **argv)
{
    parse_args(argc, argv);

    InputParser input_parser;
    input_parser.parse_input_to_generate_operations(options.dag_file_path);
    operations = input_parser.get_operations();
    operation_type_to_latency_map = input_parser.get_operation_type_to_latency_map();

    BootstrappingSegmentGenerator segment_generator(operations, false, options.num_levels);
    bootstrapping_segments = segment_generator.get_bootstrapping_segments(options.dag_file_path);
}

void BootstrapSetSelector::choose_and_output_bootstrapping_sets()
{
    while (set_index < num_sets)
    {
        reset();
        print_options();
        choose_operations_to_bootstrap();
        write_lgr_like_format();
        set_index++;
    }
}

void BootstrapSetSelector::reset()
{
    for (auto operation : operations)
    {
        operation->child_ptrs_that_receive_bootstrapped_result.clear();
    }
}

void BootstrapSetSelector::write_lgr_like_format()
{
    std::ofstream output_file;
    output_file.open(options.output_file_paths[set_index] + ".lgr");

    for (auto operation : operations)
    {
        if (operation_is_bootstrapped(operation))
        {
            output_file << "BOOTSTRAPPED( OP" << operation->id << ") 1" << std::endl;
        }
    }

    output_file.close();
}

void BootstrapSetSelector::choose_operations_to_bootstrap()
{
    auto count = 0;
    while (!bootstrapping_segments_are_satisfied(bootstrapping_segments))
    {
        max_num_segments = update_num_segments_for_every_operation();
        if (options.slack_weight[set_index] != 0)
        {
            update_all_ESTs_and_LSTs(operations, operation_type_to_latency_map);
            max_slack = update_all_slacks(operations);
        }
        if (options.urgency_weight[set_index] != 0)
        {
            update_all_bootstrap_urgencies();
        }
        choose_operation_to_bootstrap_based_on_score();
    }
}

void BootstrapSetSelector::update_all_bootstrap_urgencies()
{
    for (auto &operation : operations)
    {
        operation->bootstrap_urgency = -1;
    }

    auto count = 0;
    for (auto &segment : bootstrapping_segments)
    {
        if (segment_is_urgent(segment))
        {
            count++;
            auto segment_size = segment.size();
            for (double i = 0; i < segment_size; i++)
            {
                segment[i]->bootstrap_urgency = std::max(
                    segment[i]->bootstrap_urgency, (i + 1) / segment_size);
            }
        }
    }
    if (count == 0)
    {
        std::cout << "here" << std::endl;
    }
}

void BootstrapSetSelector::choose_operation_to_bootstrap_based_on_score()
{
    auto max_score = -1;
    OperationPtr max_score_operation;
    for (auto &operation : operations)
    {
        if (!operation_is_bootstrapped(operation))
        {
            auto score = get_score(operation);
            if (score > max_score)
            {
                max_score = score;
                max_score_operation = operation;
            }
        }
    }

    max_score_operation->child_ptrs_that_receive_bootstrapped_result = max_score_operation->child_ptrs;
}

double BootstrapSetSelector::get_score(OperationPtr operation)
{
    if (operation->num_unsatisfied_segments == 0)
    {
        return 0;
    }

    double normalized_num_segments = ((double)operation->num_unsatisfied_segments) / max_num_segments;
    double normalized_slack = ((double)operation->slack) / max_slack;

    return std::max(options.segments_weight[set_index] * normalized_num_segments +
                        options.slack_weight[set_index] * normalized_slack +
                        options.urgency_weight[set_index] * operation->bootstrap_urgency,
                    0.0);
}

int BootstrapSetSelector::update_num_segments_for_every_operation()
{
    for (auto &operation : operations)
    {
        operation->num_unsatisfied_segments = 0;
    }

    for (auto segment : bootstrapping_segments)
    {
        if (!bootstrapping_segment_is_satisfied(segment))
        {
            for (auto &operation : segment)
            {
                operation->num_unsatisfied_segments++;
            }
        }
    }

    int max = 0;

    for (auto &operation : operations)
    {
        if (operation->num_unsatisfied_segments > max)
        {
            max = operation->num_unsatisfied_segments;
        }
    }

    return max;
}

void BootstrapSetSelector::parse_args(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << help_info << std::endl;
        exit(1);
    }

    options.dag_file_path = argv[1];
    options.output_file_paths = split_string_by_character(argv[2], ',');
    num_sets = options.output_file_paths.size();

    options.num_levels = std::stoi(argv[3]);

    const std::function<int(std::string)> stoi_function = [](std::string str)
    { return std::stoi(str); };

    std::string options_string;
    for (auto i = 4; i < argc; i++)
    {
        options_string += std::string(argv[i]) + " ";
    }

    options.segments_weight = get_list_arg(options_string, "-s", "--segments-weight", help_info, num_sets, 0, stoi_function);
    options.slack_weight = get_list_arg(options_string, "-r", "--slack-weight", help_info, num_sets, 0, stoi_function);
    options.urgency_weight = get_list_arg(options_string, "-u", "--urgency-weight", help_info, num_sets, 0, stoi_function);
}

void BootstrapSetSelector::print_options()
{
    std::cout << "dag_file_path: " << options.dag_file_path << std::endl;
    std::cout << "output_file_path: " << options.output_file_paths[set_index] << std::endl;
    std::cout << "num_levels: " << options.num_levels << std::endl;
    std::cout << "segments_weight: " << options.segments_weight[set_index] << std::endl;
    std::cout << "slack_weight: " << options.slack_weight[set_index] << std::endl;
    std::cout << "urgency_weight: " << options.urgency_weight[set_index] << std::endl;
}

int main(int argc, char **argv)
{
    BootstrapSetSelector bootstrap_set_selector = BootstrapSetSelector(argc, argv);

    bootstrap_set_selector.choose_and_output_bootstrapping_sets();

    return 0;
}