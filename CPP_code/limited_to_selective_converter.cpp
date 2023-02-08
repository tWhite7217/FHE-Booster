#include "limited_to_selective_converter.h"

LimitedToSelectiveConverter::LimitedToSelectiveConverter(std::string dag_filename, std::string bootstrap_filename, std::string input_lgr_filename, int gained_levels)
{
    InputParser input_parser;
    input_parser.parse_input_to_generate_operations(dag_filename);

    operations = input_parser.get_operations();

    std::ifstream bootstrap_file(bootstrap_filename);
    bootstrap_segments = read_bootstrap_segments(bootstrap_file, operations);

    auto lgr_parser = LGRParser(input_lgr_filename, "-");
    lgr_parser.set_operations(operations);
    lgr_parser.lex();
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
            if (no_segment_relies_on_parent_child_bootstrap_pair(parent, child))
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

bool LimitedToSelectiveConverter::no_segment_relies_on_parent_child_bootstrap_pair(OperationPtr &parent, OperationPtr &child)
{
    for (auto segment : bootstrap_segments)
    {
        if (segment_relies_on_parent_child_bootstrap_pair(segment, parent, child))
        {
            return false;
        }
    }
    return true;
}

bool LimitedToSelectiveConverter::segment_relies_on_parent_child_bootstrap_pair(OperationList &segment, OperationPtr &parent, OperationPtr &child)
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

void LimitedToSelectiveConverter::write_selective_lgr_file(std::string output_lgr_filename)
{
    std::ofstream output_file;
    output_file.open(output_lgr_filename);

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
    if (argc != 6)
    {
        std::cout << "Usage: " << argv[0] << " <input_dag_file> <bootstrap_file> <input_lgr_file> <output_lgr_file> <gained_levels>" << std::endl;
        return 1;
    }

    std::string dag_filename = argv[1];
    std::string bootstrap_filename = argv[2];
    std::string input_lgr_filename = argv[3];
    std::string output_lgr_filename = argv[4];
    int gained_levels = std::atoi(argv[5]);

    auto limited_to_selective_converter =
        LimitedToSelectiveConverter(dag_filename, bootstrap_filename, input_lgr_filename, gained_levels);

    limited_to_selective_converter.remove_unnecessary_bootstrapped_results();
    limited_to_selective_converter.write_selective_lgr_file(output_lgr_filename);

    return 0;
}