#include "shared_utils.h"
#include "program.h"
#include "file_writer.h"

std::string dag_filename;
std::string lgr_filename;
std::string sched_filename;
std::string latency_filename;

const std::string help_info = R"(
Usage: ./optimal_as_sched_writer.out <dag_file>
                                     <lgr_file>
                                     <sched_file>
                                     [<options>]

Options:
  -l <file>, --latency-file=<file>
    A file describing the latencies of FHE operations on the target
    hardware. The default values can be found in program.h.)";

void parse_args(int argc, char **argv)
{
    const int minimum_arguments = 4;

    if (argc < minimum_arguments)
    {
        std::cout << help_info << std::endl;
        exit(1);
    }

    dag_filename = argv[1];
    lgr_filename = argv[2];
    sched_filename = argv[3];

    std::string options_string = utl::make_options_string(argc, argv, minimum_arguments);

    auto latency_filename = utl::get_arg(options_string, "-l", "--latency-file", help_info);
}

int main(int argc, char **argv)
{
    parse_args(argc, argv);

    Program::ConstructorInput in;
    in.dag_filename = dag_filename;
    in.latency_filename = latency_filename;
    in.bootstrap_filename = lgr_filename;

    auto program = Program(in);

    std::function<void()> main_func = [&program]()
    {
        FileWriter file_writer(program);
        file_writer.write_sched_file(sched_filename);
    };

    utl::perform_func_and_print_execution_time(main_func, "Creating .sched file from optimal lgr");

    return 0;
}