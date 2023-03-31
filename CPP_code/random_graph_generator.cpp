#include "random_graph_generator.h"

RandomGraphGenerator::RandomGraphGenerator(int argc, char **argv)
{
    parse_args(argc, argv);
    print_options();

    unsigned int seed;

    if (options.use_input_seed)
    {
        seed = read_input_seed();
        rand_gen.seed(seed);
    }
    else
    {
        seed = std::chrono::system_clock::now().time_since_epoch().count();
        rand_gen.seed(seed);
    }

    if (options.save_seed)
    {
        write_seed(seed);
    }
}

void RandomGraphGenerator::create_graph()
{
    int num_levels = utl::random_int_between(options.min_levels, options.max_levels, rand_gen);

    level_widths = get_random_level_widths(num_levels);

    auto min_constants = level_widths[0] / 2 + 1;

    num_constants = utl::random_int_between(min_constants, options.max_constants, rand_gen);

    level_ops = get_random_level_ops(level_widths);

    for (int i = num_levels - 1; i > 0; i--)
    {
        for (auto parent : level_ops[i - 1])
        {
            add_random_child_to_operation(parent, i);
        }

        for (auto operation : level_ops[i])
        {
            add_random_parents_to_operation(operation, options.probability_of_two_parents, options.constant_probability, i);
        }
    }

    for (auto operation : level_ops[0])
    {
        add_random_parents_to_operation(operation, options.probability_of_two_parents, 1, 0);
    }

    fix_constants();
}

std::vector<int> RandomGraphGenerator::get_random_level_widths(const int num_levels)
{
    std::vector<int> level_widths(num_levels, options.min_width);

    while (std::reduce(level_widths.begin(), level_widths.end()) < options.min_operations)
    {
        auto random_level = utl::random_int_between(0, num_levels - 1, rand_gen);
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

std::vector<OpVector> RandomGraphGenerator::get_random_level_ops(const std::vector<int> &level_widths)
{
    auto num_levels = level_widths.size();
    std::vector<OpVector> level_ops(num_levels);
    int i = 0;
    for (size_t j = 0; j < num_levels; j++)
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

void RandomGraphGenerator::add_random_child_to_operation(const OperationPtr &operation, int child_level)
{
    OpVector operations_without_two_parents;
    for (auto op : level_ops[child_level])
    {
        if (op->parent_ptrs.size() < 2)
        {
            operations_without_two_parents.push_back(op);
        }
    }
    int random_index = utl::random_int_between(0, operations_without_two_parents.size() - 1, rand_gen);
    operations_without_two_parents[random_index]->parent_ptrs.push_back(operation);
}

OperationPtr RandomGraphGenerator::add_random_operation_to_operations(int operation_id)
{
    int operation_type_num = rand_gen() % 13;
    OperationType operation_type;
    if (operation_type_num < 6)
    {
        operation_type = OperationType::ADD;
    }
    // else if (operation_type_num < 6)
    // {
    //     operation_type = "SUB";
    // }
    else
    {
        operation_type = OperationType::MUL;
    }
    auto new_operation = program.add_operation(Operation(operation_type, int(program.size()) + 1));
    return new_operation;
}

void RandomGraphGenerator::add_random_parents_to_operation(const OperationPtr &operation, double two_parent_probability, double constant_probability, int level)
{
    // Note: operations entering this function do not have any constant parents yet
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
        size_t num_parents = rand_decimal < two_parent_probability ? 2 : 1;

        if (num_parents == num_current_parents)
        {
            return;
        }

        std::vector<bool> parent_at_index_is_constant;

        if (level == 0)
        {
            for (size_t i = 0; i < num_parents; i++)
            {
                parent_at_index_is_constant.push_back(true);
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

        int prev_parent_id = -1;
        if (num_current_parents == 1)
        {
            prev_parent_id = operation->parent_ptrs[0]->id;
        }

        bool should_not_compare_indices = (num_parents == 1) ||
                                          (parent_at_index_is_constant[0] !=
                                           parent_at_index_is_constant[1]);

        size_t i = num_current_parents;
        while (i < num_parents)
        {
            int parent_id;
            bool parent_is_constant = parent_at_index_is_constant[i];
            if (parent_is_constant)
            {
                parent_id = utl::random_int_between(1, num_constants, rand_gen);
            }
            else if (i == 0 && num_current_parents == 0)
            {
                auto min_id = level_ops[level - 1].front()->id;
                auto max_id = level_ops[level - 1].back()->id;
                parent_id = utl::random_int_between(min_id, max_id, rand_gen);
            }
            else
            {
                auto max_id = std::reduce(level_widths.begin(), level_widths.begin() + level);
                if (max_id == 1 && (i == 1 || num_current_parents == 1))
                {
                    break;
                }
                parent_id = utl::random_int_between(1, max_id, rand_gen);
            }
            if (should_not_compare_indices || (parent_id != prev_parent_id))
            {
                if (parent_is_constant)
                {
                    operation->constant_parent_ids.push_back(parent_id);
                }
                else
                {
                    auto parent_ptr = program.get_operation_ptr_from_id(parent_id);
                    operation->parent_ptrs.push_back(parent_ptr);
                }
                prev_parent_id = parent_id;
                i++;
            }
        }

        if (!operation_is_unique(operation))
        {
            OperationPtr first_parent_ptr;
            if (num_current_parents == 1)
            {
                first_parent_ptr = operation->parent_ptrs.front();
            }
            operation->parent_ptrs.clear();
            operation->constant_parent_ids.clear();
            if (num_current_parents == 1)
            {
                operation->parent_ptrs.push_back(first_parent_ptr);
            }
        }
        else
        {
            added_parents = true;
            used_constants.insert(operation->constant_parent_ids.begin(),
                                  operation->constant_parent_ids.end());
        }
    }
}
bool RandomGraphGenerator::add_a_random_parent_type(std::vector<bool> &parent_at_index_is_constant, const double constant_probability)
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

    for (auto operation : program)
    {
        std::vector<int> new_ids;
        for (const auto const_id : operation->constant_parent_ids)
        {
            new_ids.push_back(constant_remapping[const_id]);
        }

        operation->constant_parent_ids.clear();
        for (auto id : new_ids)
        {
            operation->constant_parent_ids.push_back(id);
        }
    }
}

void RandomGraphGenerator::write_graph_to_txt_file()
{
    std::string output_filename = options.output_file_base_path + ".txt";
    std::ofstream output_file(output_filename);

    // output_file << "ADD,1" << std::endl;
    // output_file << "SUB,1" << std::endl;
    // output_file << "MUL,5" << std::endl;

    // output_file << "~" << std::endl;

    for (int i = 1; i <= num_constants; i++)
    {
        output_file << i << ",SET" << std::endl;
    }

    output_file << "~" << std::endl;

    for (auto &operation : program)
    {
        output_file << operation->id << ",";
        output_file << operation->type.to_string();

        std::string dependency_string = ",";
        for (const auto &parent : operation->parent_ptrs)
        {
            dependency_string += "c" + std::to_string(parent->id) + ",";
        }
        for (const auto parent_id : operation->constant_parent_ids)
        {
            dependency_string += "k" + std::to_string(parent_id) + ",";
        }
        dependency_string = dependency_string.substr(0, dependency_string.size() - 1);

        output_file << dependency_string << std::endl;
    }
}

bool RandomGraphGenerator::operation_is_unique(const OperationPtr &operation) const
{
    auto num_var_parents = operation->parent_ptrs.size();
    auto num_const_parents = operation->constant_parent_ids.size();

    OpSet var_parents_set;
    OpSet other_var_parents_set;
    std::unordered_set<int> const_parents_set;
    std::unordered_set<int> other_const_parents_set;

    var_parents_set.insert(operation->parent_ptrs.begin(), operation->parent_ptrs.end());
    const_parents_set.insert(operation->constant_parent_ids.begin(), operation->constant_parent_ids.end());

    for (const auto other_operation : program)
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

void RandomGraphGenerator::parse_args(int argc, char **argv)
{
    const int minimum_arguments = 7;

    if (argc < minimum_arguments)
    {
        std::cout << help_info << std::endl;
        exit(1);
    }

    options.output_file_base_path = argv[1];
    options.min_operations = std::stoi(argv[2]);
    options.min_levels = std::stoi(argv[3]);
    options.max_levels = std::stoi(argv[4]);
    options.min_width = std::stoi(argv[5]);
    options.max_width = std::stoi(argv[6]);

    std::string options_string = utl::make_options_string(argc, argv, minimum_arguments);

    std::cout << options_string << std::endl;

    options.use_input_seed = utl::arg_exists(options_string, "-i", "--input-seed");
    options.save_seed = utl::arg_exists(options_string, "-s", "--save-seed");

    auto max_constants_string = utl::get_arg(options_string, "-m", "--max-constants", help_info);
    if (!max_constants_string.empty())
    {
        options.max_constants = std::stoi(max_constants_string);
    }
    else
    {
        options.max_constants = options.max_width;
    }

    auto probability_of_two_parents_string = utl::get_arg(options_string, "-p", "--probability-of-two-parents", help_info);
    if (!probability_of_two_parents_string.empty())
    {
        options.probability_of_two_parents = std::stod(probability_of_two_parents_string);
    }

    auto constant_probability_string = utl::get_arg(options_string, "-c", "--constant-probability", help_info);
    if (!constant_probability_string.empty())
    {
        options.constant_probability = std::stod(constant_probability_string);
    }

    if ((options.min_operations < 5) ||
        (options.min_levels * options.max_width < options.min_operations) ||
        (options.max_levels * options.min_width > options.min_operations) ||
        (options.max_constants < options.max_width / 2))
    {
        std::cout << "ERROR: Input constraints violated. Now printing help info for this program." << std::endl;
        std::cout << help_info << std::endl;
        exit(1);
    }
}

void RandomGraphGenerator::print_options() const
{
    std::cout << "Generator using the following options." << std::endl;
    std::cout << "output_file_base_path: " << options.output_file_base_path << std::endl;
    std::cout << "min_operations: " << options.min_operations << std::endl;
    std::cout << "min_levels: " << options.min_levels << std::endl;
    std::cout << "max_levels: " << options.max_levels << std::endl;
    std::cout << "min_width: " << options.min_width << std::endl;
    std::cout << "max_width: " << options.max_width << std::endl;
    std::cout << "max_constants: " << options.max_constants << std::endl;
    std::cout << "probability_of_two_parents: " << options.probability_of_two_parents << std::endl;
    std::cout << "constant_probability: " << options.constant_probability << std::endl;
    std::cout << "use_input_seed: " << (options.use_input_seed ? "yes" : "no") << std::endl;
    std::cout << "save_seed: " << (options.save_seed ? "yes" : "no") << std::endl;
}

unsigned int RandomGraphGenerator::read_input_seed() const
{
    std::ifstream input_file(options.output_file_base_path + ".seed");
    std::string seed_string;
    getline(input_file, seed_string);
    return std::stoul(seed_string);
}

void RandomGraphGenerator::write_seed(unsigned int seed) const
{
    std::ofstream output_file(options.output_file_base_path + ".seed");
    output_file << seed << std::endl;
}

int main(int argc, char *argv[])
{
    RandomGraphGenerator graph_generator(argc, argv);
    graph_generator.create_graph();
    std::cout << "Graph created" << std::endl;
    graph_generator.write_graph_to_txt_file();
    std::cout << "txt created" << std::endl;
}