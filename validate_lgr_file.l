%{

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "solution_validator.h"
#include "DDGs/custom_ddg_format_parser.h"

std::vector<int> bootstrapped_operations;
std::vector<int> cores_used;
std::vector<int> start_times;
std::vector<int> bootstrap_start_times;

int max_finish_time = 0;
bool used_bootstrap_limited_model = false;
bool used_some_children_model = false;

void remove_chars_from_string( std::string &str, std::vector<char> chars_to_remove ) {
   for ( unsigned int i = 0; i < chars_to_remove.size(); ++i ) {
      str.erase( remove(str.begin(), str.end(), chars_to_remove[i]), str.end() );
   }
}

%}

%%

"BOOTSTRAPPED( OP"[0-9]+")"[ \t]+"1" {
    std::string yystring = std::string(yytext);
    int right_parenthesis = yystring.find(")");
    std::string operation_id_str = yystring.substr(16, right_parenthesis-16);
    int operation_id = std::stoi(operation_id_str);
    bootstrapped_operations.push_back(operation_id);
}

"BOOTSTRAPPED( OP"[0-9]+", OP"[0-9]+")"[ \t]+"1" {
    used_some_children_model = true;
    std::string yystring = std::string(yytext);
    int comma = yystring.find(",");
    int right_parenthesis = yystring.find(")");
    std::string operation_id_str1 = yystring.substr(16, comma-16);
    int operation_id1 = std::stoi(operation_id_str1);
    int operation2_start = comma+4;
    std::string operation_id_str2 = yystring.substr(operation2_start, right_parenthesis-operation2_start);
    int operation_id2 = std::stoi(operation_id_str2);
    bootstrapped_operations.push_back(operation_id1);
    bootstrapped_operations.push_back(operation_id2);
}

"BOOTSTRAP_FINISH_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {}

"FINISH_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {
    std::string yystring = std::string(yytext);
    remove_chars_from_string(yystring, {' ', '\t'});
    int right_parenthesis = yystring.find(")");
    int finish_time_front = right_parenthesis + 1;
    std::string finish_time_string = yystring.substr(finish_time_front, yystring.size()-finish_time_front);
    int finish_time = std::stoi(finish_time_string);
    max_finish_time = std::max(max_finish_time, finish_time);
}

"BOOTSTRAP_START_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {
    std::string yystring = std::string(yytext);
    remove_chars_from_string(yystring, {' ', '\t'});
    int right_parenthesis = yystring.find(")");
    std::string operation_id_str = yystring.substr(23, right_parenthesis-23);
    int operation_id = std::stoi(operation_id_str);
    int b_start_time_front = right_parenthesis + 1;
    std::string b_start_time_str = yystring.substr(b_start_time_front, yystring.size()-b_start_time_front);
    int b_start_time = std::stoi(b_start_time_str);
    bootstrap_start_times.push_back(b_start_time);
}

"START_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {
    std::string yystring = std::string(yytext);
    remove_chars_from_string(yystring, {' ', '\t'});
    int right_parenthesis = yystring.find(")");
    std::string operation_id_str = yystring.substr(13, right_parenthesis-13);
    int operation_id = std::stoi(operation_id_str);
    int start_time_front = right_parenthesis + 1;
    std::string start_time_str = yystring.substr(start_time_front, yystring.size()-start_time_front);
    int start_time = std::stoi(start_time_str);
    start_times.push_back(start_time);
}

"B2C( OP"[0-9]+", C"[0-9]+")"[ \t]+"1" {
    used_bootstrap_limited_model = true;
    std::string yystring = std::string(yytext);
    remove_chars_from_string(yystring, {' ', '\t'});
    int core_num_front = yystring.find("C", 3) + 1;
    int right_parenthesis = yystring.find(")");
    std::string core_num_str = yystring.substr(core_num_front, right_parenthesis-core_num_front);
    int core_num = std::stoi(core_num_str);
    cores_used.push_back(core_num);
}

. {}

"\n" {}

%%
int yywrap() { /* need this to avoid link problem */
	return 1;
}

int main(int argc, char** argv) {
	if (argc < 3) {
		std::cout << "Two files must be provided in the command line.\n";
		return 0;
	}
	yyin = fopen(argv[2], "r");
	yylex();
    InputParser input_parser = InputParser{3, true, used_some_children_model};
    input_parser.parse_input(std::string(argv[1]));
    std::vector<Operation> operations = input_parser.get_operations();
    for (unsigned int i = 0; i < operations.size(); ++i) {
        operations[i].start_time = start_times[i];
        operations[i].bootstrap_start_time = bootstrap_start_times[i];
    }
    auto solution_validator = SolutionValidator{{12,
                                                 operations,
                                                 input_parser.get_operation_type_to_latency_map(),
                                                 input_parser.get_bootstrapping_paths(),
                                                 max_finish_time,
                                                 bootstrapped_operations,
                                                 used_bootstrap_limited_model,
                                                 cores_used,
                                                 used_some_children_model}};
    solution_validator.validate_solution();
	return 0;
}
