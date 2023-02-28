#include "complete_to_selective_converter.h"

CompleteToSelectiveConverter::CompleteToSelectiveConverter(const std::string &dag_filename, const std::string &segments_filename, const std::string &bootstrap_filename)
{
    Program::ConstructorInput in;
    in.dag_filename = dag_filename;
    in.segments_filename = segments_filename;
    in.bootstrap_filename = bootstrap_filename;
    program = Program(in);
}

void CompleteToSelectiveConverter::write_selective_lgr_file(const std::string &output_lgr_filename)
{
    program.remove_unnecessary_bootstrap_pairs();
    program.file_writer->write_bootstrapping_set_to_file(output_lgr_filename);
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        std::cout << "Usage: " << argv[0] << " <input_dag_file> <segments_file> <input_lgr_file> <output_lgr_file>" << std::endl;
        return 1;
    }

    std::string dag_filename = argv[1];
    std::string segments_filename = argv[2];
    std::string input_lgr_filename = argv[3];
    std::string output_lgr_filename = argv[4];

    auto complete_to_selective_converter =
        CompleteToSelectiveConverter(dag_filename, segments_filename, input_lgr_filename);

    complete_to_selective_converter.write_selective_lgr_file(output_lgr_filename);

    return 0;
}