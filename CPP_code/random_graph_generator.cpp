#include "random_graph_generator.h"

int random_int_between(const int &min, const int &max, std::minstd_rand &rand_gen)
{
    return (rand_gen() % (max - min + 1)) + min;
}

RandomGraphGenerator::RandomGraphGenerator()
{
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    rand_gen.seed(seed);
}

void RandomGraphGenerator::create_graph(graph_generator_options new_options)
{
    options = new_options;

    int num_levels = random_int_between(options.min_levels, options.max_levels, rand_gen);

    level_widths = get_random_level_widths(num_levels);

    auto min_constants = level_widths[0] / 2 + 1;

    num_constants = random_int_between(min_constants, options.max_constants, rand_gen);

    level_ops = get_random_level_ops(level_widths);

    for (int i = num_levels - 1; i > 0; i--)
    {
        for (auto parent : level_ops[i - 1])
        {
            add_random_child_to_operation(parent, i);
        }

        for (auto operation : level_ops[i])
        {
            add_random_parents_to_operation(operation, options.probability_of_two_parents, options.probability_parent_is_a_constant, i);
        }
    }

    for (auto operation : level_ops[0])
    {
        add_random_parents_to_operation(operation, options.probability_of_two_parents, 1, 0);
    }

    add_child_ptrs_to_operation_list_with_existing_parent_ptrs(operations);
    fix_constants();
}

std::vector<int> RandomGraphGenerator::get_random_level_widths(int num_levels)
{
    std::vector<int> level_widths(num_levels, options.min_width);

    while (std::reduce(level_widths.begin(), level_widths.end()) < options.min_operations)
    {
        auto random_level = random_int_between(0, num_levels - 1, rand_gen);
        if (level_widths[random_level] < options.max_width)
        {
            level_widths[random_level]++;
            auto i = random_level;
            while (level_widths[i + 1] < ((level_widths[i] + 1) / 2))
            {
                i++;
                level_widths[i]++;
            }
        }
    }

    return level_widths;
}

std::vector<OperationList> RandomGraphGenerator::get_random_level_ops(std::vector<int> level_widths)
{
    auto num_levels = level_widths.size();
    std::vector<OperationList> level_ops(num_levels);
    int i = 0;
    for (int j = 0; j < num_levels; j++)
    {
        for (int k = 0; k < level_widths[j]; k++)
        {
            auto operation = add_random_operation_to_operations(i + 1);
            level_ops[j].push_back(operation);
            i++;
        }
    }
    return level_ops;
}

void RandomGraphGenerator::add_random_child_to_operation(OperationPtr operation, int child_level)
{
    OperationList operations_without_two_parents;
    for (auto op : level_ops[child_level])
    {
        if (op->parent_ptrs.size() < 2)
        {
            operations_without_two_parents.push_back(op);
        }
    }
    int random_index = random_int_between(0, operations_without_two_parents.size() - 1, rand_gen);
    operations_without_two_parents[random_index]->parent_ptrs.push_back(operation);
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

void RandomGraphGenerator::add_random_parents_to_operation(OperationPtr operation, double two_parent_probability, double constant_probability, int level)
{
    auto num_current_parents = operation->parent_ptrs.size();
    if (num_current_parents == 2)
    {
        return;
    }
    int tries = 0;
    bool added_parents = false;
    while (!added_parents && (num_current_parents == 0 || tries < 100))
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
        else
        {
            if (num_current_parents == 0)
            {
                parent_at_index_is_constant.push_back(false);
            }
            if (num_parents == 2)
            {
                add_a_random_parent_type(parent_at_index_is_constant, constant_probability);
            }
        }

        int i = 0;
        int prev_parent_index = -1;
        if (num_current_parents == 1)
        {
            prev_parent_index = operation->parent_ptrs[0]->id - 1;
        }

        bool should_not_compare_indices = (num_parents == 1) ||
                                          (parent_at_index_is_constant[0] !=
                                           parent_at_index_is_constant[1]);

        while (i < (num_parents - num_current_parents))
        {
            int parent_index;
            bool parent_is_constant = parent_at_index_is_constant[i];
            if (parent_is_constant)
            {
                parent_index = random_int_between(1, num_constants, rand_gen);
            }
            else if (i == 0 && num_current_parents == 0)
            {
                auto min_index = level_ops[level - 1].front()->id - 1;
                auto max_index = level_ops[level - 1].back()->id - 1;
                parent_index = random_int_between(min_index, max_index, rand_gen);
            }
            else
            {
                auto max_index = std::reduce(level_widths.begin(), level_widths.begin() + level) - 1;
                if (max_index == 0 && (i == 1 || num_current_parents == 1))
                {
                    break;
                }
                parent_index = random_int_between(0, max_index, rand_gen);
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
            if (num_current_parents == 0)
            {
                operation->parent_ptrs.clear();
            }
            else if (operation->parent_ptrs.size() == 2)
            {
                operation->parent_ptrs.pop_back();
            }
            operation->constant_parent_ids.clear();
        }
        else
        {
            added_parents = true;
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

void RandomGraphGenerator::write_graph_to_txt_file(std::string output_filename)
{
    std::ofstream output_file(output_filename);

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
    if (argc != 10)
    {
        std::cout << "Usage: ./graph_generator <min_num_operations> <min_levels> <max_levels> <min_width> <max_width> <max_constants> <probability_of_two_parents> <probability_parent_is_a_constant> <output_file_base_path>" << std::endl;
        return 1;
    }

    auto min_operations = std::stoi(argv[1]);
    auto min_levels = std::stoi(argv[2]);
    auto max_levels = std::stoi(argv[3]);
    auto min_width = std::stoi(argv[4]);
    auto max_width = std::stoi(argv[5]);
    auto max_constants = std::stoi(argv[6]);
    auto probability_of_two_parents = std::stod(argv[7]);
    auto probability_parent_is_a_constant = std::stod(argv[8]);
    std::string output_file_base_path = argv[9];

    if ((min_operations < 5) ||
        (min_levels * max_width < min_operations) ||
        (max_levels * min_width > min_operations) ||
        (max_constants < max_width / 2))
    {
        std::cout << "Inputs must meet the following constraints." << std::endl;
        std::cout << "min_operations >= 5" << std::endl;
        std::cout << "min_levels * max_width >= min_operations" << std::endl;
        std::cout << "max_levels * min_width <= min_operations" << std::endl;
        std::cout << "max_constants >= max_width / 2" << std::endl;
        exit(1);
    }

    auto options = RandomGraphGenerator::graph_generator_options{
        min_operations,
        min_levels,
        max_levels,
        min_width,
        max_width,
        max_constants,
        probability_of_two_parents,
        probability_parent_is_a_constant};

    RandomGraphGenerator graph_generator;
    graph_generator.create_graph(options);
    std::cout << "Graph created" << std::endl;
    graph_generator.write_graph_to_txt_file(output_file_base_path + ".txt");
    std::cout << "txt created" << std::endl;
}