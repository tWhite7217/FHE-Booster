BIN = ./bin

shared_depenedencies := $(BIN)/shared_utils.o $(BIN)/custom_ddg_format_parser.o $(BIN)/bootstrapping_path_generator.o

lgr_parser_dependencies := $(BIN)/lex.o $(BIN)/LGRParser.o

CPP_FLAGS = -std=c++17 -O3

all: solution_validator.out list_scheduler.out limited_to_selective_converter.out input_file_processor.out graph_generator.out tgff_to_unprocessed_lingo.out

$(BIN)/shared_utils.o: shared_utils.cpp shared_utils.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ shared_utils.cpp

$(BIN)/custom_ddg_format_parser.o: custom_ddg_format_parser.cpp custom_ddg_format_parser.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ custom_ddg_format_parser.cpp

$(BIN)/bootstrapping_path_generator.o: bootstrapping_path_generator.cpp bootstrapping_path_generator.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ bootstrapping_path_generator.cpp

$(BIN)/lex.o: LGRParser.l Makefile
	flexc++ LGRParser.l
	g++ $(CPP_FLAGS) -c -o $@ lex.cc

$(BIN)/LGRParser.o: LGRParser.cpp LGRParser.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ LGRParser.cpp

$(BIN)/solution_validator.o: solution_validator.cpp solution_validator.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ solution_validator.cpp

solution_validator.out: $(BIN)/solution_validator.o $(shared_depenedencies) $(lgr_parser_dependencies)
	g++ $(CPP_FLAGS) -o $@ $(BIN)/solution_validator.o $(shared_depenedencies) $(lgr_parser_dependencies)

$(BIN)/list_scheduler.o: list_scheduler.cpp list_scheduler.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ list_scheduler.cpp

list_scheduler.out: $(BIN)/list_scheduler.o $(shared_depenedencies) $(lgr_parser_dependencies)
	g++ $(CPP_FLAGS) -o $@ $(BIN)/list_scheduler.o $(shared_depenedencies) $(lgr_parser_dependencies)

$(BIN)/limited_to_selective_converter.o: limited_to_selective_converter.cpp limited_to_selective_converter.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ limited_to_selective_converter.cpp

limited_to_selective_converter.out: $(BIN)/limited_to_selective_converter.o $(shared_depenedencies) $(lgr_parser_dependencies)
	g++ $(CPP_FLAGS) -o $@ $(BIN)/limited_to_selective_converter.o $(shared_depenedencies) $(lgr_parser_dependencies)

$(BIN)/input_file_processor.o: input_file_processor.cpp Makefile
	g++ $(CPP_FLAGS) -c -o $@ input_file_processor.cpp

input_file_processor.out: $(BIN)/input_file_processor.o $(shared_depenedencies)
	g++ $(CPP_FLAGS) -o $@ $(BIN)/input_file_processor.o $(shared_depenedencies)

$(BIN)/random_graph_generator.o: random_graph_generator.cpp random_graph_generator.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ random_graph_generator.cpp

random_graph_generator.out: $(BIN)/random_graph_generator.o $(BIN)/shared_utils.o
	g++ $(CPP_FLAGS) -o $@ $(BIN)/random_graph_generator.o $(BIN)/shared_utils.o

$(BIN)/txt_to_vcg.o: txt_to_vcg.cpp Makefile
	g++ $(CPP_FLAGS) -c -o $@ txt_to_vcg.cpp

txt_to_vcg.out: $(BIN)/txt_to_vcg.o $(BIN)/custom_ddg_format_parser.o $(BIN)/shared_utils.o
	g++ $(CPP_FLAGS) -o $@ $(BIN)/txt_to_vcg.o $(BIN)/custom_ddg_format_parser.o $(BIN)/shared_utils.o

tgff_to_unprocessed_lingo.out: tgff_to_unprocessed_lingo.l Makefile
	flex -o lex.yy.c tgff_to_unprocessed_lingo.l
	g++ $(CPP_FLAGS) -o $@ lex.yy.c

clean:
	rm lex.cc LGRParserBase.h lex.yy.c *.out bin/*