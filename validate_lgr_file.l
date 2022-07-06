%{

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

#include "solution_validator.h"
#include "DDGs/custom_ddg_format_parser.h"

std::vector<int> bootstrapped_operations;
std::vector<int> start_times;
std::vector<int> bootstrap_start_times;
std::map<int, int> cores_used;

int max_finish_time = 0;
bool used_bootstrap_limited_model = false;
bool used_some_children_model = false;

void remove_chars_from_string( std::string &str, std::vector<char> chars_to_remove ) {
   for ( unsigned int i = 0; i < chars_to_remove.size(); ++i ) {
      str.erase( remove(str.begin(), str.end(), chars_to_remove[i]), str.end() );
   }
}

int extract_number_from_string(std::string str, size_t start_index, size_t end_index) {
    auto num_digits = end_index - start_index;
    std::string num_as_string = str.substr(start_index, num_digits);
    int num = std::stoi(num_as_string);
    return num;
}

int get_first_operation_id(char *yytext) {
    std::string yystring = std::string(yytext);
    auto operation_start_index = yystring.find("OP") + 2;
    auto operation_end_index = yystring.find(",");
    if (operation_end_index == std::string::npos) {
        operation_end_index = yystring.find(")");
    }
    return extract_number_from_string(yystring, operation_start_index, operation_end_index);
}

int get_second_operation_id(char *yytext) {
    std::string yystring = std::string(yytext);
    auto operation_start_index = yystring.rfind("OP") + 2;
    auto operation_end_index = yystring.find(")");
    return extract_number_from_string(yystring, operation_start_index, operation_end_index);
}

int get_core_num(char *yytext) {
    std::string yystring = std::string(yytext);
    auto core_start_index = yystring.find(", C") + 3;
    auto core_end_index = yystring.find(")");
    return extract_number_from_string(yystring, core_start_index, core_end_index);
}

int get_time(char *yytext) {
    std::string yystring = std::string(yytext);
    remove_chars_from_string(yystring, {' ', '\t'});
    auto time_start_index = yystring.find(")") + 1;
    auto time_end_index = yystring.size();
    return extract_number_from_string(yystring, time_start_index, time_end_index);
}

%}

%%

"BOOTSTRAPPED( OP"[0-9]+")"[ \t]+"1" {
    int operation_id = get_first_operation_id(yytext);
    bootstrapped_operations.push_back(operation_id);
}

"BOOTSTRAPPED( OP"[0-9]+", OP"[0-9]+")"[ \t]+"1" {
    used_some_children_model = true;
    int operation_id1 = get_first_operation_id(yytext);
    int operation_id2 = get_second_operation_id(yytext);
    bootstrapped_operations.push_back(operation_id1);
    bootstrapped_operations.push_back(operation_id2);
}

"BOOTSTRAP_FINISH_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {}

"FINISH_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {
    int finish_time = get_time(yytext);
    max_finish_time = std::max(max_finish_time, finish_time);
}

"BOOTSTRAP_START_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {
    int b_start_time = get_time(yytext);
    bootstrap_start_times.push_back(b_start_time);
}

"START_TIME( OP"[0-9]+")"[ \t]+[0-9]+ {
    int start_time = get_time(yytext);
    start_times.push_back(start_time);
}

"B2C( OP"[0-9]+", C"[0-9]+")"[ \t]+"1" {
    used_bootstrap_limited_model = true;
    int operation_id = get_first_operation_id(yytext);
    int core_num = get_core_num(yytext);
    cores_used[operation_id] = core_num;
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
    for (unsigned int i = 0; i < operations.size(); i++) {
        operations[i].start_time = start_times[i];
        operations[i].bootstrap_start_time = bootstrap_start_times[i];
    }
    for (auto [operation_id, core_num] : cores_used) {
        operations[operation_id-1].core_num = core_num;
    }
    auto solution_validator = SolutionValidator{{12,
                                                 operations,
                                                 input_parser.get_operation_type_to_latency_map(),
                                                 input_parser.get_bootstrapping_paths(),
                                                 max_finish_time,
                                                 bootstrapped_operations,
                                                 used_bootstrap_limited_model,
                                                 used_some_children_model}};
    solution_validator.validate_solution();
	return 0;
}
