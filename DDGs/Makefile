tgff_to_unprocessed_lingo: tgff_to_unprocessed_lingo.l
	flex -o lex.yy.c tgff_to_unprocessed_lingo.l; g++ -o tgff_to_unprocessed_lingo lex.yy.c -std=c++17; rm lex.yy.c

input_file_processor: input_file_processor.cpp custom_ddg_format_parser.cpp custom_ddg_format_parser.h ../shared_utils.h
	g++ -o input_file_processor input_file_processor.cpp custom_ddg_format_parser.cpp shared_utils.cpp -std=c++17