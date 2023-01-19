#include "random_graph_generator.h"

RandomGraphGenerator::RandomGraphGenerator()
{
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    rand_gen.seed(seed);
}

void RandomGraphGenerator::create_graph(graph_generator_options options)
{
    int num_starting_operations = rand_gen() % options.max_num_starting_operations + 1;
    num_constants = rand_gen() % (options.max_num_constants - options.min_num_constants) + options.min_num_constants;

    for (int i = 0; i < num_starting_operations; i++)
    {
        auto operation = add_random_operation_to_operations(i + 1);

        add_random_parents_to_operation(operation, options.probability_of_two_parents, 1);
    }

    for (int i = num_starting_operations; i < options.num_operations; i++)
    {
        auto operation = add_random_operation_to_operations(i + 1);

        add_random_parents_to_operation(operation, options.probability_of_two_parents, options.probability_parent_is_a_constant);
    }

    add_child_ptrs_to_operation_list_with_existing_parent_ptrs(operations);
    fix_constants();
}

OperationPtr RandomGraphGenerator::add_random_operation_to_operations(int operation_id)
{
    int operation_type_num = rand_gen() % 13;
    std::string operation_type;
    if (operation_type_num < 6)
    {
        operation_type = "ADD";
    }
    // else if (operation_type_num < 6)
    // {
    //     operation_type = "SUB";
    // }
    else
    {
        operation_type = "MUL";
    }
    operations.emplace_back(new Operation{operation_type, operation_id});
    return operations.back();
}

void RandomGraphGenerator::add_random_parents_to_operation(OperationPtr operation, double two_parent_probability, double constant_probability)
{
    while (operation_has_no_parents(operation))
    {
        double rand_decimal = ((double)rand_gen()) / RAND_MAX;
        int num_parents = rand_decimal < two_parent_probability ? 2 : 1;

        std::vector<bool> parent_at_index_is_constant;

        if (constant_probability == 1)
        {
            for (int i = 0; i < num_parents; i++)
            {
                parent_at_index_is_constant.push_back(true);
            }
        }
        else if (operations.size() == 2)
        {
            auto parent_is_constant = add_a_random_parent_type(parent_at_index_is_constant, constant_probability);
            if (num_parents == 2)
            {
                if (parent_is_constant)
                {
                    add_a_random_parent_type(parent_at_index_is_constant, constant_probability);
                }
                else
                {
                    parent_at_index_is_constant.push_back(true);
                }
            }
        }
        else
        {
            parent_at_index_is_constant.push_back(false);
            if (num_parents == 2)
            {
                add_a_random_parent_type(parent_at_index_is_constant, constant_probability);
            }
        }

        int i = 0;
        int prev_parent_index = -1;

        bool should_not_compare_indices = (num_parents == 1) ||
                                          (parent_at_index_is_constant[0] !=
                                           parent_at_index_is_constant[1]);

        while (i < num_parents)
        {
            int parent_index;
            bool parent_is_constant = parent_at_index_is_constant[i];
            if (parent_is_constant)
            {
                parent_index = rand_gen() % num_constants + 1;
            }
            else
            {
                parent_index = rand_gen() % (operations.size() - 1);
            }
            if (should_not_compare_indices || (parent_index != prev_parent_index))
            {
                if (parent_is_constant)
                {
                    operation->constant_parent_ids.push_back(parent_index);
                }
                else
                {
                    auto parent_ptr = operations[parent_index];
                    operation->parent_ptrs.push_back(parent_ptr);
                }
                prev_parent_index = parent_index;
                i++;
            }
        }

        if (!operation_is_unique(operation))
        {
            operation->parent_ptrs.clear();
            operation->constant_parent_ids.clear();
        }
        else
        {
            used_constants.insert(operation->constant_parent_ids.begin(),
                                  operation->constant_parent_ids.end());
        }
    }
}
bool RandomGraphGenerator::add_a_random_parent_type(std::vector<bool> &parent_at_index_is_constant, const double &constant_probability)
{
    double rand_decimal = ((double)rand_gen()) / RAND_MAX;
    bool parent_is_constant = rand_decimal < constant_probability;
    parent_at_index_is_constant.push_back(parent_is_constant);
    return parent_is_constant;
}

void RandomGraphGenerator::fix_constants()
{
    std::map<int, int> constant_remapping;
    int expected_constant_num = 0;
    for (int constant : used_constants)
    {
        expected_constant_num++;
        constant_remapping[constant] = expected_constant_num;
    }
    num_constants = expected_constant_num;

    for (auto operation : operations)
    {
        for (auto &const_id : operation->constant_parent_ids)
        {
            const_id = constant_remapping[const_id];
        }
    }
}

void RandomGraphGenerator::write_graph_to_txt_file(std::string output_file_path)
{
    std::ofstream output_file(output_file_path);

    output_file << "ADD,1" << std::endl;
    output_file << "SUB,1" << std::endl;
    output_file << "MUL,5" << std::endl;

    output_file << "~" << std::endl;

    for (int i = 1; i <= num_constants; i++)
    {
        output_file << i << ",SET" << std::endl;
    }

    output_file << "~" << std::endl;

    for (auto &operation : operations)
    {
        output_file << operation->id << ",";
        output_file << operation->type;

        std::string dependency_string = ",";
        for (auto parent : operation->parent_ptrs)
        {
            dependency_string += "c" + std::to_string(parent->id) + ",";
        }
        for (auto parent_id : operation->constant_parent_ids)
        {
            dependency_string += "k" + std::to_string(parent_id) + ",";
        }
        dependency_string = dependency_string.substr(0, dependency_string.size() - 1);

        output_file << dependency_string << std::endl;
    }
}

bool RandomGraphGenerator::operation_is_unique(OperationPtr operation)
{
    auto num_var_parents = operation->parent_ptrs.size();
    auto num_const_parents = operation->constant_parent_ids.size();

    std::unordered_set<OperationPtr> var_parents_set;
    std::unordered_set<OperationPtr> other_var_parents_set;
    std::unordered_set<int> const_parents_set;
    std::unordered_set<int> other_const_parents_set;

    var_parents_set.insert(operation->parent_ptrs.begin(), operation->parent_ptrs.end());
    const_parents_set.insert(operation->constant_parent_ids.begin(), operation->constant_parent_ids.end());

    for (auto other_operation : operations)
    {
        if (operation != other_operation)
        {
            if ((operation->type != other_operation->type) ||
                (num_var_parents != other_operation->parent_ptrs.size()) ||
                (num_const_parents != other_operation->constant_parent_ids.size()))
            {
                continue;
            }

            other_var_parents_set.clear();
            other_const_parents_set.clear();

            other_var_parents_set.insert(other_operation->parent_ptrs.begin(), other_operation->parent_ptrs.end());
            other_const_parents_set.insert(other_operation->constant_parent_ids.begin(), other_operation->constant_parent_ids.end());

            if ((var_parents_set == other_var_parents_set) &&
                (const_parents_set == other_const_parents_set))
            {
                return false;
            }
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 8)
    {
        std::cout << "Usage: ./graph_generator <num_operations> <max_num_starting_operations> <min_num_constants> <max_num_constants> <probability_of_two_parents> <probability_parent_is_a_constant> <output_file_base_path>" << std::endl;
        return 1;
    }

    auto num_operations = std::stoi(argv[1]);
    auto max_num_starting_operations = std::stoi(argv[2]);
    auto min_num_constants = std::stoi(argv[3]);
    auto max_num_constants = std::stoi(argv[4]);
    auto probability_of_two_parents = std::stod(argv[5]);
    auto probability_parent_is_a_constant = std::stod(argv[6]);
    std::string output_file_base_path = argv[7];

    if (min_num_constants < 1)
    {
        std::cout << "Must be at least one constant." << std::endl;
        exit(1);
    }

    auto options = RandomGraphGenerator::graph_generator_options{
        num_operations,
        max_num_starting_operations,
        min_num_constants,
        max_num_constants,
        probability_of_two_parents,
        probability_parent_is_a_constant};

    RandomGraphGenerator graph_generator;
    graph_generator.create_graph(options);
    std::cout << "Graph created" << std::endl;
    graph_generator.write_graph_to_txt_file(output_file_base_path + ".txt");
    std::cout << "txt created" << std::endl;
}