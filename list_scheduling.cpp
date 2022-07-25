#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <queue>
#include <algorithm>
#include <boost/range/irange.hpp>
#include <boost/range/adaptor/reversed.hpp>

void update_earliest_start_time(Instruction *instruction)
{
    for (auto earlier_id : instruction->block_dependences)
    {
        Instruction *earlier_instruction = get_instruction_pointer_from_id(earlier_id);
        int possible_est =
            earlier_instruction->earliest_start_time +
            earlier_instruction->latency;
        if (possible_est > instruction->earliest_start_time)
        {
            instruction->earliest_start_time = possible_est;
        }
    }
}

int get_earliest_end_time_of_basic_block(auto basic_block)
{
    int earliest_end_time = 0;
    for (auto instruction : basic_block)
    {
        int possible_end_time = instruction.earliest_start_time + instruction.latency;
        if (possible_end_time > earliest_end_time)
        {
            earliest_end_time = possible_end_time;
        }
    }
    return earliest_end_time;
}

void update_latest_start_time(Instruction *instruction, int earliest_end_time)
{
    if (instruction->block_dependent_instructions.empty())
    {
        instruction->latest_start_time =
            earliest_end_time - instruction->latency;
    }
    else
    {
        for (auto later_id : instruction->block_dependent_instructions)
        {
            Instruction *later_instruction = get_instruction_pointer_from_id(later_id);
            int possible_lst =
                later_instruction->latest_start_time -
                later_instruction->latency;
            if (possible_lst < instruction->latest_start_time)
            {
                instruction->latest_start_time = possible_lst;
            }
        }
    }
}

void update_ESTs_and_LSTs_of_basic_block(int i)
{
    auto num_instructions = int(basic_blocks[i].size());

    auto forward_range = boost::irange(num_instructions);
    auto backward_range = boost::adaptors::reverse(forward_range);

    for (auto j : forward_range)
    {
        update_earliest_start_time(get_instruction_pointer_from_id({i, j}));
    }

    int earliest_end_time = get_earliest_end_time_of_basic_block(basic_blocks[i]);

    for (auto j : backward_range)
    {
        update_latest_start_time(get_instruction_pointer_from_id({i, j}), earliest_end_time);
    }
}

void update_all_ESTs_and_LSTs()
{
    for (auto i : boost::irange(basic_blocks.size()))
    {
        update_ESTs_and_LSTs_of_basic_block(i);
    }
}

void update_all_ranks()
{
    for (auto i : boost::irange(basic_blocks.size()))
    {
        auto num_instructions = int(basic_blocks[i].size());
        for (auto j : boost::irange(num_instructions))
        {
            Instruction *instruction = get_instruction_pointer_from_id({i, j});
            instruction->rank = instruction->latest_start_time - instruction->earliest_start_time;
        }
    }
}

std::list<InstructionID> get_basic_block_priority_list(auto basic_block)
{
    std::list<InstructionID> priority_list;
    for (auto instruction_to_place : basic_block)
    {
        auto i = priority_list.begin();
        bool inserted = false;
        for (auto id : priority_list)
        {
            Instruction *instruction_in_list = get_instruction_pointer_from_id(id);
            if (instruction_in_list->rank > instruction_to_place.rank)
            {
                priority_list.insert(i, instruction_to_place.id);
                inserted = true;
                break;
            }
            i++;
        }
        if (!inserted)
        {
            priority_list.push_back(instruction_to_place.id);
        }
    }
    return priority_list;
}

std::map<InstructionID, int> initialize_pred_count(auto basic_block)
{
    std::map<InstructionID, int> pred_count;
    for (auto instruction : basic_block)
    {
        pred_count[instruction.id] = instruction.block_dependences.size();
    }
    return pred_count;
}

std::set<InstructionID> initialize_ready_instructions(auto pred_count)
{
    std::set<InstructionID> ready_instructions;
    for (const auto [id, count] : pred_count)
    {
        if (count == 0)
        {
            ready_instructions.insert(id);
        }
    }
    return ready_instructions;
}

void create_reordered_basic_blocks()
{
    for (auto block : basic_blocks)
    {
        std::list<InstructionID> priority_list = get_basic_block_priority_list(block);
        std::map<InstructionID, int> pred_count = initialize_pred_count(block);
        std::set<InstructionID> ready_instructions = initialize_ready_instructions(pred_count);
        reordered_basic_blocks.emplace_back();
        while (!ready_instructions.empty())
        {
            for (const auto id : priority_list)
            {
                if (ready_instructions.find(id) != ready_instructions.end())
                {
                    Instruction *instruction = get_instruction_pointer_from_id(id);
                    reordered_basic_blocks.back().push_back(*instruction);
                    ready_instructions.erase(id);
                    for (auto dependent_id : instruction->block_dependent_instructions)
                    {
                        pred_count[dependent_id] -= 1;
                        if (pred_count[dependent_id] == 0)
                        {
                            ready_instructions.insert(dependent_id);
                        }
                    }
                    break;
                }
            }
        }
    }
    for (auto block : reordered_basic_blocks)
    {
        for (auto instruction : block)
        {
            std::cout << instruction.text << "\n";
        }
    }
}

bool instruction_depends_on_other_instruction(Instruction instruction, Instruction other_instruction)
{
    auto dependences = instruction.general_dependences;
    return std::find(dependences.begin(), dependences.end(), other_instruction.id) != dependences.end();
}

Instruction get_next_ready_instruction(auto &instruction_queue, Instruction other_running_instruction, int running_time_left)
{
    if (!instruction_queue.empty())
    {
        Instruction next_instruction = instruction_queue.front();
        if (running_time_left > 0)
        {
            if (instruction_depends_on_other_instruction(next_instruction, other_running_instruction))
            {
                return Instruction();
            }
        }
        instruction_queue.pop();
        return next_instruction;
    }
    return Instruction();
}

int get_latency(auto blocks)
{
    int clock_cycle = 0;
    Instruction running_instruction_1;
    int running_time_left_1 = 0;
    Instruction running_instruction_2;
    int running_time_left_2 = 0;
    std::queue<Instruction> instruction_queue;
    for (auto block : blocks)
    {
        for (auto instruction : block)
        {
            if (instruction.latency != 0)
            {
                instruction_queue.push(instruction);
            }
        }
    }
    while (!instruction_queue.empty())
    {
        running_time_left_1 = std::max(0, running_time_left_1 - 1);
        running_time_left_2 = std::max(0, running_time_left_1 - 1);
        if (running_time_left_1 == 0)
        {
            running_instruction_1 = get_next_ready_instruction(instruction_queue, running_instruction_2, running_time_left_2);
            running_time_left_1 = running_instruction_1.latency;
        }

        if (running_time_left_2 == 0)
        {
            running_instruction_2 = get_next_ready_instruction(instruction_queue, running_instruction_1, running_time_left_1);
            running_time_left_2 = running_instruction_2.latency;
        }

        clock_cycle++;
    }
    clock_cycle += std::max(running_time_left_1, running_time_left_2);
    return clock_cycle;
}

int main()
{
    if (argc != 4)
    {
        std::cout << "Usage: " << argv[0] << " <input_file> <output_file> <allow_bootstrapping_to_only_some_children>" << std::endl;
        return 1;
    }

    dag_file_path = std::string{argv[1]};
    bootstrapped_tasks_file_path = std::string{argv[1]};
    output_file_path = std::string{argv[2]};
    allow_bootstrapping_to_only_some_children = (std::string(argv[3]) == "True");

    addition_divider = 1;

    InputParser input_parser = InputParser{bootstrapping_path_length, false, used_some_children_model, addition_divider};
    input_parser.parse_input_file(dag_file_path);

    

    update_all_ESTs_and_LSTs();
    update_all_ranks();
    create_reordered_basic_blocks();
    int original_schedule_latency = get_latency(basic_blocks);
    printf("original schedule latency: %d\n", original_schedule_latency);
    int new_schedule_latency = get_latency(reordered_basic_blocks);
    printf("new schedule latency: %d\n", new_schedule_latency);
    print_c_code(basic_blocks);
    return 0;
}
