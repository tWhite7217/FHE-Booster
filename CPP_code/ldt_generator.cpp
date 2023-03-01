#include "program.h"
#include "file_writer.h"

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        std::cout << "Usage: " << argv[0] << " <dag_file> <segments_file> <output_file> <\"COMPLETE\" or \"SELECTIVE\">" << std::endl;
        return 1;
    }

    bool using_selective_model = (std::string(argv[4]) == "SELECTIVE");

    Program::ConstructorInput in;
    in.dag_filename = argv[1];
    in.segments_filename = argv[2];
    in.b_mode = using_selective_model ? BootstrapMode::SELECTIVE : BootstrapMode::COMPLETE;
    auto program = Program(in);

    std::string output_filename = argv[3];
    auto file_writer = FileWriter(std::ref(program));

    std::function<void()> write_func = [file_writer, output_filename]()
    { file_writer.write_ldt_info_to_file(output_filename); };

    utl::perform_func_and_print_execution_time(write_func, "Writing LDT to file");
}