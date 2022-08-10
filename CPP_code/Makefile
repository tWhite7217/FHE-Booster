shared_header_files := shared_utils.h custom_ddg_format_parser.h bootstrapping_path_generator.h
shared_source_files := shared_utils.cpp custom_ddg_format_parser.cpp bootstrapping_path_generator.cpp

shared_depenedencies := $(shared_header_files) $(shared_source_files)

lgr_parser_source_files := LGRParser.cpp lex.cc

lgr_parser_dependencies := LGRParser.l LGRParser.cpp LGRParser.h

all: solution_validator list_scheduler input_file_processor tgff_to_unprocessed_lingo

solution_validator: solution_validator.cpp solution_validator.h $(shared_depenedencies) $(lgr_parser_dependencies)
	flexc++ LGRParser.l
	g++ -o solution_validator solution_validator.cpp $(shared_source_files) $(lgr_parser_source_files) -std=c++17

list_scheduler: list_scheduling.cpp list_scheduling.h $(shared_depenedencies) $(lgr_parser_dependencies)
	flexc++ LGRParser.l
	g++ -o list_scheduler list_scheduling.cpp $(shared_source_files) $(lgr_parser_source_files) -std=c++17

input_file_processor: input_file_processor.cpp $(shared_depenedencies)
	g++ -o input_file_processor input_file_processor.cpp $(shared_source_files) -std=c++17

graph_generator: graph_generator.cpp graph_generator.h shared_utils.cpp shared_utils.h
	g++ -o graph_generator graph_generator.cpp shared_utils.cpp -std=c++17

tgff_to_unprocessed_lingo: tgff_to_unprocessed_lingo.l
	flex -o lex.yy.c tgff_to_unprocessed_lingo.l
	g++ -o tgff_to_unprocessed_lingo lex.yy.c -std=c++17

clean:
	rm lex.cc LGRParserBase.h list_scheduler solution_validator lex.yy.c