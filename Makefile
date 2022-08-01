shared_header_files := shared_utils.h DDGs/custom_ddg_format_parser.h LGRParser.h bootstrapping_path_generator.h
shared_source_files := shared_utils.cpp DDGs/custom_ddg_format_parser.cpp LGRParser.cpp bootstrapping_path_generator.cpp lex.cc

shared_depenedencies := LGRParser.l $(shared_header_files) $(shared_source_files)

validate_lgr_file: solution_validator.cpp solution_validator.h $(shared_depenedencies)
	flexc++ LGRParser.l; g++ -o solution_validator solution_validator.cpp $(shared_source_files) -std=c++17

list_scheduler: list_scheduling.cpp list_scheduling.h $(shared_depenedencies)
	flexc++ LGRParser.l; g++ -o list_scheduler list_scheduling.cpp $(shared_source_files) -std=c++17

clean:
	rm lex.cc LGRParserBase.h list_scheduler solution_validator