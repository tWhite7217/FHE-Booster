%{

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

std::string output;

std::vector<std::string> bootstrapped_operations;
std::vector<std::string> bootstrap_start_times;
std::vector<std::string> cores_used;

int max_finish_time = 0;

void remove_chars_from_string( std::string &str, std::vector<char> chars_to_remove ) {
   for ( unsigned int i = 0; i < chars_to_remove.size(); ++i ) {
      str.erase( remove(str.begin(), str.end(), chars_to_remove[i]), str.end() );
   }
}

void create_output() {
    for ( unsigned int i = 0; i < bootstrapped_operations.size(); ++i ) {
        output += bootstrapped_operations[i];
        output += ",";
    }
    output += "\n";
    output += std::to_string( max_finish_time );
    output += "\n";
    for ( unsigned int i = 0; i < cores_used.size(); ++i ) {
        output += cores_used[i];
        output += ",";
    }
    output += "\n";
    for ( unsigned int i = 0; i < bootstrap_start_times.size(); ++i ) {
        output += bootstrap_start_times[i];
        output += ",";
    }
}

%}

%%

"BOOTSTRAPPED( OP"[0-9]+")"[ \t]+"1" {
    std::string yystring = std::string(yytext);
    int right_parenthesis = yystring.find(")");
    std::string operation = yystring.substr(14, right_parenthesis-14);
    bootstrapped_operations.push_back(operation);
}

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
    std::string operation = yystring.substr(21, right_parenthesis-21);
    if(std::find(bootstrapped_operations.begin(), bootstrapped_operations.end(), operation) != bootstrapped_operations.end()) {
        int b_start_time_front = right_parenthesis + 1;
        std::string b_start_time = yystring.substr(b_start_time_front, yystring.size()-b_start_time_front);
        bootstrap_start_times.push_back(b_start_time);
    }
}

"B2C( OP"[0-9]+", C"[0-9]+")"[ \t]+"1" {
    std::string yystring = std::string(yytext);
    remove_chars_from_string(yystring, {' ', '\t'});
    int core_num_front = yystring.find("C", 3) + 1;
    int right_parenthesis = yystring.find(")");
    std::string core_num = yystring.substr(core_num_front, right_parenthesis-core_num_front);
    cores_used.push_back(core_num);
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
