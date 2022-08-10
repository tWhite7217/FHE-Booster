%{

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

enum class OperationType {
    ADD,
    MUL,
};

struct Operation {
    int id;
    OperationType type;
    std::vector<std::shared_ptr<Operation>> depends_on;

    Operation(int id) : id(id) {}
};

std::vector<std::shared_ptr<Operation>> operations;

std::string output;

std::unordered_map<OperationType, int> operation_latencies = {
    {OperationType::ADD, 1},
    {OperationType::MUL, 5},
};

std::unordered_map<OperationType, std::string> operation_strings = {
    {OperationType::ADD, "ADD"},
    {OperationType::MUL, "MUL"},
};

void create_output() {
    for (auto& [operation_type, latency] : operation_latencies) {
        output += operation_strings[operation_type] + "," + std::to_string(latency) + "\n";
    }

    output += "~\n";
    
    for (auto& operation : operations) {
        output += std::to_string(operation->id) + ",";
        output += operation_strings[operation->type];
        if (operation->depends_on.size() > 0) {
            output += ",";
            for (auto& dependency : operation->depends_on) {
                output += std::to_string(dependency->id) + ",";
            }
            output = output.substr(0, output.size() - 1);
        }
        output += "\n";
    }
}

%}

%%

"TASK t0_"[0-9]+[ \t]"TYPE "[0-9]+ {
    operations.emplace_back(std::make_shared<Operation>(operations.size()+1));
    std::string yystring = std::string{yytext};
    std::string type_string = yystring.substr(yystring.find("TYPE") + 5, yystring.size());
    int type_num = std::stoi(type_string);
    operations.back()->type = static_cast<OperationType>(type_num);
}

"FROM t0_"[0-9]+"  TO  t0_"[0-9]+ {
    std::string yystring = std::string{yytext};
    std::string from_string = yystring.substr(8, yystring.find("  TO"));
    int from_num = std::stoi(from_string);
    std::string to_string = yystring.substr(yystring.rfind("t0_") + 3, yystring.size());
    int to_num = std::stoi(to_string);
    operations[to_num]->depends_on.push_back(operations[from_num]);
}

. {}

"\n" {}

%%
int yywrap() { /* need this to avoid link problem */
	return 1;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "A file must be provided in the command line.\n";
		return 0;
	}
	yyin = fopen(argv[1], "r");
	yylex();
    create_output();
	std::cout << output;
	return 0;
}
