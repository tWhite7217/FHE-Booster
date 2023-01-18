#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <fstream>
#include <cassert>
#include <sstream>

enum Op { EMUL, CMUL, EADD, CADD, ESUB, CSUB, EINV, BOOT };

class Node {
  std::vector<std::string> inputs;
  std::string output_wire;
  Op operation;
  public:
    std::vector<std::string> get_inputs() {
      return inputs;
    }
    std::string get_output() {
      return output_wire;
    }
    Op get_op() {
      return operation;
    }
    void print_node() {
      std::cout << "Op: " << operation << std::endl;
      std::cout << "Input(s): ";
      for (uint64_t i = 0; i < inputs.size(); i++) {
        std::cout << inputs[i] << " ";
      }
      std::cout << std::endl;
      std::cout << "Output: " << output_wire << std::endl;
    }
    void set_operation(std::string in_op) {
      if (in_op == "MUL" || in_op == "ADD" || in_op == "SUB") {
        assert(inputs.size() == 2);
        bool cond1 = inputs[0].find('p') == std::string::npos;
        bool cond2 = inputs[1].find('p') == std::string::npos;
        if (cond1 & cond2) {
          operation = in_op == "MUL" ? EMUL : (in_op == "ADD" ? EADD : ESUB);
        }
        else if (cond1 ^ cond2) {
          operation = in_op == "MUL" ? CMUL : (in_op == "ADD" ? CADD : CSUB);
        }
        else {
          std::cout << "ERROR: Ptxt-ptxt ops are not supported!" << std::endl;
          std::cout << "Trace: " << in_op << " " << inputs[0] << " " 
                    << inputs[1] << " " << output_wire << std::endl;
          exit(-1);
        }
      }
      else if (in_op == "BOOT" || in_op == "INV") {
        assert(inputs.size() == 1);
        bool ctxt_check = inputs[0].find('p') == std::string::npos;
        if (ctxt_check) {
          operation = in_op == "BOOT" ? BOOT : EINV;
        }
        else {
          std::cout << "ERROR: Invalid operation on ptxt!" << std::endl;
          std::cout << "Trace: " << in_op << " " << inputs[0] << " " 
                    << " " << output_wire << std::endl;
          exit(-1);
        }
      }
      else {
        std::cout << "ERROR: Unsupported operation: " << in_op << "!" << std::endl;
        exit(-1);
      }
    }
    Node(){}
    Node(std::string in_op, std::string in_out, std::string in1, std::string in2) {
      assert(in_out != "");

      output_wire = in_out;
      inputs.clear();
      if (in1 != "") {
        inputs.push_back(in1);
      }
      if (in2 != "") {
        inputs.push_back(in2);
      }
      set_operation(in_op);
    }
};

std::vector<std::queue<Node*>> parse_schedule(std::string sched, int num_workers);