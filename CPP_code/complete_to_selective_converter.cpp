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
    std::string output_lgr_filename = argv[4];

    std::ofstream log_file(output_lgr_filename + ".log");

    size_t num_pairs_removed;
    size_t total_num_pairs;

    std::function<void()> main_func = [&program, &output_lgr_filename, &num_pairs_removed, &total_num_pairs]()
    {
        std::function<void()> conversion_func = [&program, &num_pairs_removed, &total_num_pairs]()
        { program.remove_unnecessary_bootstrap_pairs(num_pairs_removed, total_num_pairs); };

        utl::perform_func_and_print_execution_time(
            conversion_func, "Removing unnecessary bootstrap pairs");

        auto file_writer = FileWriter(std::ref(program));

        std::function<void()> write_func = [&file_writer, &output_lgr_filename]()
        { file_writer.write_bootstrapping_set_to_file(output_lgr_filename); };

        utl::perform_func_and_print_execution_time(
            write_func, "Writing converted bootstrap set to file");
    };

    utl::perform_func_and_print_execution_time(main_func, log_file);

    log_file << num_pairs_removed << std::endl;
    size_t num_remaining_pairs = total_num_pairs - num_pairs_removed;
    log_file << num_remaining_pairs << std::endl;
    // log_file << total_num_pairs << std::endl;

    return 0;
}