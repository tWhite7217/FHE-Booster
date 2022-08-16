#include "limited_to_selective_converter.h"

LimitedToSelectiveConverter::LimitedToSelectiveConverter(std::string input_dag_file_path, std::string input_lgr_file_path)
{
    InputParser input_parser;
    input_parser.parse_input_to_generate_operations(input_dag_file_path);

    operations = input_parser.get_operations();

    auto lgr_parser = LGRParser(input_lgr_file_path, "-");
    lgr_parser.set_operations(operations);
    lgr_parser.lex();

    auto bootstrapping_path_generator = BootstrappingPathGenerator(operations, true);
    bootstrapping_paths = bootstrapping_path_generator.generate_bootstrapping_paths();
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
            if (no_path_relies_on_parent_child_bootstrapping_pair(parent, child))
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

bool LimitedToSelectiveConverter::no_path_relies_on_parent_child_bootstrapping_pair(OperationPtr &parent, OperationPtr &child)
{
    for (auto path : bootstrapping_paths)
    {
        if (path_relies_on_parent_child_bootstrapping_pair(path, parent, child))
        {
            return false;
        }
    }
    return true;
}

bool LimitedToSelectiveConverter::path_relies_on_parent_child_bootstrapping_pair(OperationList &path, OperationPtr &parent, OperationPtr &child)
{
    for (auto i = 0; i < path.size() - 1; i++)
    {
        auto other_parent = path[i];
        auto other_child = path[i + 1];
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
    if (argc != 4)
    {
        std::cout << "Usage: " << argv[0] << " <input_dag_file> <input_lgr_file> <output_lgr_file>" << std::endl;
        return 1;
    }

    std::string input_dag_file_path = argv[1];
    std::string input_lgr_file_path = argv[2];
    std::string output_lgr_file_path = argv[3];

    auto limited_to_selective_converter =
        LimitedToSelectiveConverter(input_dag_file_path, input_lgr_file_path);

    limited_to_selective_converter.remove_unnecessary_bootstrapped_results();
    limited_to_selective_converter.write_selective_lgr_file(output_lgr_file_path);

    return 0;
}