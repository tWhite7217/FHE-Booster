make_csv_from_results: make_csv_from_results.l
	flex -o lex.yy.c make_csv_from_results.l; g++ -o make_csv_from_results lex.yy.c -std=c++17; rm lex.yy.c