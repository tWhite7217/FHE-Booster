# make_csv_from_results: make_csv_from_results.l
	# flex -o lex.yy.c make_csv_from_results.l; g++ -o make_csv_from_results lex.yy.c -std=c++17; rm lex.yy.c

validate_lgr_file: validate_lgr_file.l solution_validator.cpp solution_validator.h ../shared_utils.h ../DDGs/custom_ddg_format_parser.cpp ../DDGs/custom_ddg_format_parser.h
	flex -o lex.yy.c validate_lgr_file.l; g++ -o validate_lgr_file lex.yy.c solution_validator.cpp ../DDGs/custom_ddg_format_parser.cpp -std=c++17; rm lex.yy.c