#include "parser.hpp"

ScheduleInfo parse_schedule(CommandLineOptions options)
{
  ScheduleInfo sched_info;
  std::unordered_set<std::string> all_inputs;
  std::ifstream sched_file(options.input_filename);
  if (!sched_file)
  {
    std::cout << "Error opening file: " << options.input_filename << std::endl;
    exit(-1);
  }
  sched_info.circuit.resize(options.num_threads);
  std::string line;
  std::array<std::string, 3> operands;
  std::string operation;
  std::set<std::string> outputs;
  std::map<std::string, std::string> bootstrap_out_to_in;
  while (getline(sched_file, line))
  {
    std::stringstream ss(line);
    int idx = 0;
    while (getline(ss, line, ' '))
    {
      if (idx == 0)
      {
        operation = line;
      }
      else if (line.find("t") != std::string::npos)
      {
        line.erase(0, 1);
        int thread_idx = std::stoi(line) - 1;
        idx = 0;

        if (options.mode == ALAP && operation == "BOOT")
        {
          std::cout << "ERROR: ALAP mode schedules cannot contain bootstrapping operations." << std::endl;
          exit(-1);
        }
        else
        {
          if (operation == "ADD" || operation == "SUB" || operation == "MUL")
          {
            Node *tmp = new Node(operation, operands[0], operands[1], operands[2]);
            sched_info.circuit[thread_idx].push(tmp);
            all_inputs.insert(operands[1]);
            all_inputs.insert(operands[2]);
            sched_info.dependent_outputs[operands[1]].insert(operands[0]);
            sched_info.dependent_outputs[operands[2]].insert(operands[0]);
          }
          else
          {
            Node *tmp = new Node(operation, operands[0], operands[1], "");
            sched_info.circuit[thread_idx].push(tmp);
            all_inputs.insert(operands[1]);
            sched_info.dependent_outputs[operands[1]].insert(operands[0]);
          }
          if (outputs.count(operands[0]))
          {
            std::cout << "ERROR: Schedules must maintain SSA form." << std::endl;
            std::cout << "Ciphertext " << operands[0] << " is the output of multiple operations." << std::endl;
            exit(-1);
          }
          else
          {
            outputs.insert(operands[0]);
          }
        }
        continue;
      }
      else
      {
        operands[idx - 1] = line;
      }
      idx++;
    }
  }
  sched_file.close();
  if (options.mode == ALAP)
  {
    find_bootstrap_candidates(sched_info);
  }
  sched_info.initial_inputs = all_inputs;
  for (auto output : outputs)
  {
    sched_info.initial_inputs.erase(output);
  }
  return sched_info;
}

void find_bootstrap_candidates(ScheduleInfo &sched_info)
{
  for (auto core_schedule : sched_info.circuit)
  {
    while (!core_schedule.empty())
    {
      auto inputs = core_schedule.front()->get_inputs();
      auto op_type = core_schedule.front()->get_op();
      if (op_type == EMUL)
      {
        sched_info.bootstrap_candidates.insert(inputs.begin(), inputs.end());
      }
      else if (op_type == CMUL)
      {
        bool input0_is_ctxt = core_schedule.front()->get_inputs()[0].find('p') == std::string::npos;
        size_t i = input0_is_ctxt ? 0 : 1;
        sched_info.bootstrap_candidates.insert(inputs[i]);
      }
      core_schedule.pop();
    }
  }
}