#include "parser.hpp"

using namespace std;

vector<queue<Node*>> parse_schedule(string sched, int num_workers, bool do_T2_style_bootstrapping, std::unordered_set<string> &all_inputs) {
  ifstream sched_file(sched);
  if(!sched_file) {
    cout << "Error opening file: " << sched << endl;
    exit(-1);
  }
  vector<queue<Node*>> circuit(num_workers);
  string line;
  string* operands = new string[3];
  string operation; 
  std::set<string> outputs;
  std::map<string, string> bootstrap_out_to_in;
  while (getline(sched_file, line)) {
    stringstream ss(line);
    int idx = 0;
    while (getline(ss, line, ' ')) {
      if (idx == 0) {
        operation = line;
      }
      else if (line.find("t") != string::npos) {
        line.erase(0,1);
        int thread_idx = std::stoi(line)-1;
        idx = 0;
        
        if (do_T2_style_bootstrapping && operation == "BOOT"){
          bootstrap_out_to_in[operands[0]] = operands[1];
        }
        else {
          if (operation == "ADD" || operation == "SUB" || operation == "MUL") {
            Node* tmp = new Node(operation, operands[0], operands[1], operands[2]);
            circuit[thread_idx].push(tmp);
          }
          else {
            Node* tmp = new Node(operation, operands[0], operands[1], "");
            circuit[thread_idx].push(tmp);
          }
          if (outputs.count(operands[0])) {
            std::cout << "ERROR: Schedules must maintain SSA form." << std::endl;
            std::cout << "Ciphertext " << operands[0] << " is the output of multiple operations." << std::endl;
            exit(-1);
          } else {
            outputs.insert(operands[0]);
          }
        }
        continue;
      }
      else {
        operands[idx-1] = line;
      }
      idx++;
    }
  }
  delete [] operands;
  sched_file.close();
  if (do_T2_style_bootstrapping) {
    fix_circuit_io(circuit, bootstrap_out_to_in, all_inputs);
  }
  return circuit;
}

void fix_circuit_io(vector<queue<Node*>> circuit, const std::map<std::string, std::string> &bootstrap_out_to_in, std::unordered_set<std::string> &all_inputs) {
  for (auto schedule : circuit) {
    while (!schedule.empty()) {
      auto inputs = schedule.front()->get_inputs();
      vector<string> new_inputs(inputs.size());
      for (auto i = 0; i < inputs.size(); i++) {
        if (bootstrap_out_to_in.count(inputs[i])) {
          new_inputs[i] = bootstrap_out_to_in.at(inputs[i]);
        } else {
          new_inputs[i] = inputs[i];
        }
      }
      all_inputs.insert(new_inputs.begin(), new_inputs.end());
      schedule.front()->set_inputs(new_inputs);
      schedule.pop();
    }
  }
}