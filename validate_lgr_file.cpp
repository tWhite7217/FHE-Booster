#include "LGRParser.h"
#include "solution_validator.h"
#include "DDGs/custom_ddg_format_parser.h"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Two files must be provided in the command line.\n";
        return 0;
    }
    LGRParser lgr_parser = LGRParser{argv[2], "-"};
    lgr_parser.lex();
    const int bootstrapping_path_length = 3;
    const int addition_divider = 1;
    InputParser input_parser = InputParser{bootstrapping_path_length, true, lgr_parser.used_some_children_model, addition_divider};
    input_parser.parse_input(std::string(argv[1]));
    std::vector<Operation> operations = input_parser.get_operations();
    if (operations.size() != lgr_parser.start_times.size())
    {
        std::cout << "The number of operations in the input file does not match the number of operations in the LGR file.\n";
        return 0;
    }
    for (unsigned int i = 0; i < operations.size(); i++)
    {
        operations[i].start_time = lgr_parser.start_times[i];
    }
    if (lgr_parser.bootstrap_start_times.size() > 0)
    {
        for (unsigned int i = 0; i < operations.size(); i++)
        {
            operations[i].bootstrap_start_time = lgr_parser.bootstrap_start_times[i];
        }
    }
    for (auto [operation_id, core_num] : lgr_parser.cores_used)
    {
        operations[operation_id - 1].core_num = core_num;
    }
    auto solution_validator = SolutionValidator{{12,
                                                 operations,
                                                 input_parser.get_operation_type_to_latency_map(),
                                                 input_parser.get_bootstrapping_paths(),
                                                 lgr_parser.max_finish_time,
                                                 lgr_parser.bootstrapped_operations,
                                                 lgr_parser.used_bootstrap_limited_model,
                                                 lgr_parser.used_some_children_model}};
    solution_validator.validate_solution();
    return 0;
}
