validate_lgr_file: solution_validator.cpp solution_validator.h shared_utils.h DDGs/custom_ddg_format_parser.cpp DDGs/custom_ddg_format_parser.h LGRParser.l LGRParser.h LGRParser.cpp
	flexc++ LGRParser.l; g++ -o solution_validator lex.cc solution_validator.cpp DDGs/custom_ddg_format_parser.cpp LGRParser.cpp -std=c++17

list_scheduler: list_scheduling.cpp shared_utils.h DDGs/custom_ddg_format_parser.cpp DDGs/custom_ddg_format_parser.h LGRParser.l LGRParser.h LGRParser.cpp
	flexc++ LGRParser.l; g++ -o list_scheduler list_scheduling.cpp lex.cc DDGs/custom_ddg_format_parser.cpp LGRParser.cpp -std=c++17