shared_header_files := shared_utils.h custom_ddg_format_parser.h bootstrapping_path_generator.h
shared_source_files := shared_utils.cpp custom_ddg_format_parser.cpp bootstrapping_path_generator.cpp

shared_depenedencies := $(shared_header_files) $(shared_source_files)

lgr_parser_source_files := LGRParser.cpp lex.cc

lgr_parser_dependencies := LGRParser.l LGRParser.cpp LGRParser.h

CPP_FLAGS = -std=c++17 -O3

all: solution_validator list_scheduler input_file_processor tgff_to_unprocessed_lingo

solution_validator: solution_validator.cpp solution_validator.h $(shared_depenedencies) $(lgr_parser_dependencies) Makefile
	flexc++ LGRParser.l
	g++ -o solution_validator solution_validator.cpp $(shared_source_files) $(lgr_parser_source_files) $(CPP_FLAGS)

list_scheduler: list_scheduling.cpp list_scheduling.h $(shared_depenedencies) $(lgr_parser_dependencies) Makefile
	flexc++ LGRParser.l
	g++ -o list_scheduler list_scheduling.cpp $(shared_source_files) $(lgr_parser_source_files) $(CPP_FLAGS)

limited_to_selective_converter: limited_to_selective_converter.cpp limited_to_selective_converter.h $(shared_depenedencies) $(lgr_parser_dependencies) Makefile
	flexc++ LGRParser.l
	g++ -o limited_to_selective_converter limited_to_selective_converter.cpp $(shared_source_files) $(lgr_parser_source_files) $(CPP_FLAGS)

input_file_processor: input_file_processor.cpp $(shared_depenedencies) Makefile
	g++ -o input_file_processor input_file_processor.cpp $(shared_source_files) $(CPP_FLAGS)

graph_generator: graph_generator.cpp graph_generator.h shared_utils.cpp shared_utils.h Makefile
	g++ -o graph_generator graph_generator.cpp shared_utils.cpp $(CPP_FLAGS)

tgff_to_unprocessed_lingo: tgff_to_unprocessed_lingo.l Makefile
	flex -o lex.yy.c tgff_to_unprocessed_lingo.l
	g++ -o tgff_to_unprocessed_lingo lex.yy.c $(CPP_FLAGS)

clean:
	rm lex.cc LGRParserBase.h list_scheduler solution_validator lex.yy.c