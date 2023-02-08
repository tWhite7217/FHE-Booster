#include "limited_to_selective_converter.h"

LimitedToSelectiveConverter::LimitedToSelectiveConverter(std::string input_dag_file_path, std::string input_lgr_file_path, int gained_levels)
{
    InputParser input_parser;
    input_parser.parse_input_to_generate_operations(input_dag_file_path);

    operations = input_parser.get_operations();

    auto lgr_parser = LGRParser(input_lgr_file_path, "-");
    lgr_parser.set_operations(operations);
    lgr_parser.lex();

    auto bootstrapping_segment_generator = BootstrappingSegmentGenerator(operations, true, gained_levels);
    bootstrapping_segments = bootstrapping_segment_generator.get_bootstrapping_segments(input_dag_file_path);
    // bootstrapping_segments = bootstrapping_segment_generator.generate_bootstrapping_segments();
}

void LimitedToSelectiveConverter::remove_unnecessary_bootstrapped_results()
{
    int num_removed = 0;
    for (auto parent : operations)
    {
        auto child_it = parent->child_ptrs_that_receive_bootstrapped_result.begin();
        while (child_it != parent->child_ptrs_that_receive_bootstrapped_result.end())
        {
            auto child = *child_it;
            if (no_segment_relies_on_parent_child_bootstrapping_pair(parent, child))
            {
                child_it = parent->child_ptrs_that_receive_bootstrapped_result.erase(child_it);
                num_removed++;
            }
            else
            {
                child_it++;
            }
        }
    }
    std::cout << "Removed " << num_removed << " unnecessary bootstrapped results." << std::endl;
}

bool LimitedToSelectiveConverter::no_segment_relies_on_parent_child_bootstrapping_pair(OperationPtr &parent, OperationPtr &child)
{
    for (auto segment : bootstrapping_segments)
    {
        if (segment_relies_on_parent_child_bootstrapping_pair(segment, parent, child))
        {
            return false;
        }
    }
    return true;
}

bool LimitedToSelectiveConverter::segment_relies_on_parent_child_bootstrapping_pair(OperationList &segment, OperationPtr &parent, OperationPtr &child)
{
    for (auto i = 0; i < segment.size() - 1; i++)
    {
        auto other_parent = segment[i];
        auto other_child = segment[i + 1];
        if (other_parent != parent || other_child != child)
        {
            if (vector_contains_element(other_parent->child_ptrs_that_receive_bootstrapped_result, other_child))
            {
                return false;
            }
        }
    }
    return true;
}

void LimitedToSelectiveConverter::write_selective_lgr_file(std::string output_lgr_file_path)
{
    std::ofstream output_file;
    output_file.open(output_lgr_file_path);

    for (auto operation : operations)
    {
        for (auto child : operation->child_ptrs_that_receive_bootstrapped_result)
        {
            output_file << "BOOTSTRAPPED( OP" << operation->id << ", OP" << child->id << ") 1" << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        std::cout << "Usage: " << argv[0] << " <input_dag_file> <input_lgr_file> <output_lgr_file> <gained_levels>" << std::endl;
        return 1;
    }

    std::string input_dag_file_path = argv[1];
    std::string input_lgr_file_path = argv[2];
    std::string output_lgr_file_path = argv[3];
    int gained_levels = std::atoi(argv[4]);

    auto limited_to_selective_converter =
        LimitedToSelectiveConverter(input_dag_file_path, input_lgr_file_path, gained_levels);

    limited_to_selective_converter.remove_unnecessary_bootstrapped_results();
    limited_to_selective_converter.write_selective_lgr_file(output_lgr_file_path);

    return 0;
}