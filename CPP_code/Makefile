BIN = ./bin

primitive_dependencies := $(BIN)/operation_type.o $(BIN)/operation.o $(BIN)/bootstrap_segment.o $(BIN)/program.o $(BIN)/custom_ddg_format_parser.o $(BIN)/lex.o $(BIN)/LGRParser.o

shared_depenedencies := $(BIN)/shared_utils.o $(primitive_dependencies)

CPP_FLAGS = -std=c++17 -O3

all: bootstrap_segments_generator.out bootstrap_set_selector.out list_scheduler.out limited_to_selective_converter.out random_graph_generator.out #ldt_generator.out

$(BIN)/shared_utils.o: shared_utils.cpp shared_utils.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ shared_utils.cpp

$(BIN)/operation_type.o: operation_type.cpp operation_type.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ operation_type.cpp

$(BIN)/operation.o: operation.cpp operation.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ operation.cpp

$(BIN)/bootstrap_segment.o: bootstrap_segment.cpp bootstrap_segment.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ bootstrap_segment.cpp

$(BIN)/program.o: program.cpp program.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ program.cpp

$(BIN)/custom_ddg_format_parser.o: custom_ddg_format_parser.cpp custom_ddg_format_parser.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ custom_ddg_format_parser.cpp

$(BIN)/lex.o: LGRParser.l Makefile
	flexc++ LGRParser.l
	g++ $(CPP_FLAGS) -c -o $@ lex.cc

$(BIN)/LGRParser.o: LGRParser.cpp LGRParser.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ LGRParser.cpp

$(BIN)/bootstrap_segments_generator.o: bootstrap_segments_generator.cpp bootstrap_segments_generator.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ bootstrap_segments_generator.cpp

bootstrap_segments_generator.out: $(BIN)/bootstrap_segments_generator.o $(shared_depenedencies)
	g++ $(CPP_FLAGS) -o $@ $(BIN)/bootstrap_segments_generator.o $(shared_depenedencies)

$(BIN)/bootstrap_set_selector.o: bootstrap_set_selector.cpp bootstrap_set_selector.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ bootstrap_set_selector.cpp

bootstrap_set_selector.out: $(BIN)/bootstrap_set_selector.o $(shared_depenedencies)
	g++ $(CPP_FLAGS) -o $@ $(BIN)/bootstrap_set_selector.o $(shared_depenedencies)

$(BIN)/list_scheduler.o: list_scheduler.cpp list_scheduler.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ list_scheduler.cpp

list_scheduler.out: $(BIN)/list_scheduler.o $(shared_depenedencies)
	g++ $(CPP_FLAGS) -o $@ $(BIN)/list_scheduler.o $(shared_depenedencies)

$(BIN)/limited_to_selective_converter.o: limited_to_selective_converter.cpp limited_to_selective_converter.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ limited_to_selective_converter.cpp

limited_to_selective_converter.out: $(BIN)/limited_to_selective_converter.o $(shared_depenedencies)
	g++ $(CPP_FLAGS) -o $@ $(BIN)/limited_to_selective_converter.o $(shared_depenedencies)

# $(BIN)/ldt_generator.o: ldt_generator.cpp Makefile
# 	g++ $(CPP_FLAGS) -c -o $@ ldt_generator.cpp

# ldt_generator.out: $(BIN)/ldt_generator.o $(shared_depenedencies)
# 	g++ $(CPP_FLAGS) -o $@ $(BIN)/ldt_generator.o $(shared_depenedencies)

$(BIN)/random_graph_generator.o: random_graph_generator.cpp random_graph_generator.h Makefile
	g++ $(CPP_FLAGS) -c -o $@ random_graph_generator.cpp

random_graph_generator.out: $(BIN)/random_graph_generator.o $(BIN)/shared_utils.o $(primitive_dependencies)
	g++ $(CPP_FLAGS) -o $@ $(BIN)/random_graph_generator.o $(BIN)/shared_utils.o $(primitive_dependencies)

$(BIN)/txt_to_vcg.o: txt_to_vcg.cpp Makefile
	g++ $(CPP_FLAGS) -c -o $@ txt_to_vcg.cpp

txt_to_vcg.out: $(BIN)/txt_to_vcg.o $(BIN)/shared_utils.o $(primitive_dependencies)
	g++ $(CPP_FLAGS) -o $@ $(BIN)/txt_to_vcg.o $(BIN)/shared_utils.o $(primitive_dependencies)

clean:
	rm lex.cc LGRParserBase.h lex.yy.c *.out bin/*