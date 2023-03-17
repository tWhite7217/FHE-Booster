#include <iostream>

#include "program.h"
#include "file_writer.h"

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        std::cout << "Usage: " << argv[0] << " <input_dag_file> <segments_file> <input_lgr_file> <output_lgr_file>" << std::endl;
        return 1;
    }

    Program::ConstructorInput in;
    in.dag_filename = argv[1];
    in.segments_filename = argv[2];
    in.bootstrap_filename = argv[3];
    auto program = Program(in);
    program.set_boot_mode(BootstrapMode::SELECTIVE);

    std::function<void()> conversion_func = [&program]()
    { program.remove_unnecessary_bootstrap_pairs(); };

    utl::perform_func_and_print_execution_time(
        conversion_func, "Removing unnecessary bootstrap pairs");

    std::string output_lgr_filename = argv[4];
    auto file_writer = FileWriter(std::ref(program));

    std::function<void()> write_func = [&file_writer, &output_lgr_filename]()
    { file_writer.write_bootstrapping_set_to_file(output_lgr_filename); };

    utl::perform_func_and_print_execution_time(
        write_func, "Writing converted bootstrap set to file");

    return 0;
}