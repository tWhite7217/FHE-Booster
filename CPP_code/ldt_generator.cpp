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

    std::string output_filename = argv[3];

    auto program = Program(in);

    auto file_writer = FileWriter(std::ref(program));
    file_writer.write_ldt_info_to_file(output_filename);
}