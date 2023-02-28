#include "parser.hpp"

ScheduleInfo ScheduleParser::parse(const CommandLineOptions &options)
{
  reset_parser(options);

  auto sched_file = open_sched_file();

  std::string line_str;
  while (getline(sched_file, line_str))
  {
    parse_line(line_str);
  }
  sched_file.close();

  if (options.mode == ALAP)
  {
    find_bootstrap_candidates();
  }

  sched_info.initial_inputs = get_initial_inputs();

  return sched_info;
}

std::ifstream ScheduleParser::open_sched_file()
{
  std::ifstream sched_file(options.input_filename);
  if (!sched_file)
  {
    std::cout << "Error opening file: " << options.input_filename << std::endl;
    exit(-1);
  }
  return sched_file;
}

void ScheduleParser::reset_parser(const CommandLineOptions &options)
{
  this->options = options;
  sched_info = ScheduleInfo();
  all_inputs.clear();
  outputs.clear();
}

void ScheduleParser::parse_line(const std::string &line_str)
{
  std::vector<std::string> line = utl::split_string_by_character(line_str, ' ');
  std::string op_type = line[0];
  std::string output_key = line[1];
  std::string input_key1 = line[2];
  std::string input_key2;
  int thread_idx;

  if (options.mode == ALAP && op_type == "BOOT")
  {
    std::cout << "ERROR: ALAP mode schedules cannot contain bootstrap operations." << std::endl;
    exit(-1);
  }
  else if (op_type == "ADD" || op_type == "SUB" || op_type == "MUL")
  {
    input_key2 = line[3];
    thread_idx = get_thread_idx(line[4]);
  }
  else
  {
    thread_idx = get_thread_idx(line[3]);
  }

  EngineOperationPtr tmp = EngineOperationPtr(
      new EngineOperation(op_type, output_key, input_key1, input_key2));

  if (thread_idx >= sched_info.circuit.size())
  {
    sched_info.circuit.resize(thread_idx + 1);
  }

  sched_info.circuit[thread_idx].push_back(tmp);

  all_inputs.insert(tmp->get_inputs()[0]);
  sched_info.dependent_outputs[input_key1].insert(output_key);

  if (!input_key2.empty())
  {
    all_inputs.insert(tmp->get_inputs()[1]);
    sched_info.dependent_outputs[input_key2].insert(output_key);
  }

  if (outputs.count(output_key))
  {
    std::cout << "ERROR: Schedules must maintain SSA form." << std::endl;
    std::cout << "Ciphertext " << output_key << " is the output of multiple operations." << std::endl;
    exit(-1);
  }
  else
  {
    outputs.insert(output_key);
  }
}

int ScheduleParser::get_thread_idx(std::string thread_str) const
{
  thread_str.erase(0, 1);
  return std::stoi(thread_str) - 1;
}

void ScheduleParser::find_bootstrap_candidates()
{
  for (const auto &core_schedule : sched_info.circuit)
  {
    for (const auto &op : core_schedule)
    {
      auto inputs = op->get_inputs();
      auto op_type = op->get_op_type();
      if (op_type == CC_MUL)
      {
        sched_info.bootstrap_candidates.insert(inputs[0].key);
        sched_info.bootstrap_candidates.insert(inputs[1].key);
      }
      else if (op_type == CP_MUL)
      {
        size_t i = inputs[0].is_ctxt ? 0 : 1;
        sched_info.bootstrap_candidates.insert(inputs[i].key);
      }
    }
  }
}

std::unordered_set<EngineOpInput, EngineOpInput::hash> ScheduleParser::get_initial_inputs() const
{
  std::unordered_set<EngineOpInput, EngineOpInput::hash> initial_inputs;
  for (const auto &input : all_inputs)
  {
    if (!outputs.count(input.key))
    {
      initial_inputs.insert(input);
    }
  }
  return initial_inputs;
}