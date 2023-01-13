#include "parser.hpp"

using namespace std;

vector<queue<Node*>> parse_schedule(string sched, int num_workers) {
  ifstream sched_file(sched);
  if(!sched_file) {
    cout << "Error opening file: " << sched << endl;
    exit(-1);
  }
  vector<queue<Node*>> circuit(num_workers);
  string line;
  string* operands = new string[3];
  string operation; 
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
        if (operation == "ADD" || operation == "SUB" || operation == "MUL") {
          Node* tmp = new Node(operation, operands[0], operands[1], operands[2]);
          circuit[thread_idx].push(tmp);
        }
        else {
          Node* tmp = new Node(operation, operands[0], operands[1], "");
          circuit[thread_idx].push(tmp);
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
  return circuit;
}