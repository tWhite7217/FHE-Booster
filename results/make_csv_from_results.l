%{

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

std::string output;

std::vector<std::string> bootstrapped_operations;

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
    std::string finish_time_string = yystring.substr(yystring.find(")")+1, yystring.size());
    int finish_time = std::stoi(finish_time_string);
    max_finish_time = std::max(max_finish_time, finish_time);
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
