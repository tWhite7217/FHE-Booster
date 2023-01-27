#include "parser.hpp"

using namespace std;

ScheduleInfo parse_schedule(string sched, int num_workers, bool do_T2_style_bootstrapping)
{
  ScheduleInfo sched_info;
  std::unordered_set<string> all_inputs;
  ifstream sched_file(sched);
  if (!sched_file)
  {
    cout << "Error opening file: " << sched << endl;
    exit(-1);
  }
  sched_info.circuit.resize(num_workers);
  string line;
  string *operands = new string[3];
  string operation;
  std::set<string> outputs;
  std::map<string, string> bootstrap_out_to_in;
  while (getline(sched_file, line))
  {
    stringstream ss(line);
    int idx = 0;
    while (getline(ss, line, ' '))
    {
      if (idx == 0)
      {
        operation = line;
      }
      else if (line.find("t") != string::npos)
      {
        line.erase(0, 1);
        int thread_idx = std::stoi(line) - 1;
        idx = 0;

        if (do_T2_style_bootstrapping && operation == "BOOT")
        {
          bootstrap_out_to_in[operands[0]] = operands[1];
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
  delete[] operands;
  sched_file.close();
  if (do_T2_style_bootstrapping)
  {
    all_inputs.clear();
    sched_info.dependent_outputs.clear();
    fix_circuit_io(sched_info, bootstrap_out_to_in, all_inputs);
  }
  sched_info.initial_inputs = all_inputs;
  for (auto output : outputs)
  {
    sched_info.initial_inputs.erase(output);
  }
  return sched_info;
}

void fix_circuit_io(ScheduleInfo &sched_info, const std::map<std::string, std::string> &bootstrap_out_to_in, std::unordered_set<std::string> &all_inputs)
{
  for (auto core_schedule : sched_info.circuit)
  {
    while (!core_schedule.empty())
    {
      auto inputs = core_schedule.front()->get_inputs();
      auto output = core_schedule.front()->get_output();
      auto op_type = core_schedule.front()->get_op();
      vector<string> new_inputs(inputs.size());
      for (auto i = 0; i < inputs.size(); i++)
      {
        if (bootstrap_out_to_in.count(inputs[i]))
        {
          new_inputs[i] = bootstrap_out_to_in.at(inputs[i]);
        }
        else
        {
          new_inputs[i] = inputs[i];
        }
        sched_info.dependent_outputs[new_inputs[i]].insert(output);
        if (op_type == EMUL)
        {
          sched_info.bootstrap_candidates.insert(new_inputs.begin(), new_inputs.end());
        }
        else if (op_type == CMUL)
        {
          bool whichPtxt = core_schedule.front()->get_inputs()[0].find('p') == std::string::npos;
          size_t i = whichPtxt ? 0 : 1;
          sched_info.bootstrap_candidates.insert(new_inputs[i]);
        }
      }
      all_inputs.insert(new_inputs.begin(), new_inputs.end());
      core_schedule.front()->set_inputs(new_inputs);
      core_schedule.pop();
    }
  }
}