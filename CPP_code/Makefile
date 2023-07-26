CXX = g++-10

BIN = ./bin

primitive_dependencies = $(BIN)/operation_type.o $(BIN)/operation.o $(BIN)/bootstrap_segment.o $(BIN)/program.o $(BIN)/file_parser.o $(BIN)/lex.o $(BIN)/LGRParser.o $(BIN)/file_writer.o

shared_depenedencies = $(BIN)/shared_utils.o $(primitive_dependencies)

o_dependencies = lgrp.cc Makefile

CPP_FLAGS = -std=c++20 -O3 -Werror -Wextra -flto

all: bootstrap_segments_generator.out bootstrap_set_selector.out list_scheduler.out complete_to_selective_converter.out random_graph_generator.out ldt_generator.out txt_to_vcg.out lgr_to_sched.out

$(BIN)/shared_utils.o: shared_utils.cpp shared_utils.h $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ shared_utils.cpp

$(BIN)/operation_type.o: operation_type.cpp operation_type.h $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ operation_type.cpp

$(BIN)/operation.o: operation.cpp operation.h $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ operation.cpp

$(BIN)/bootstrap_segment.o: bootstrap_segment.cpp bootstrap_segment.h $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ bootstrap_segment.cpp

$(BIN)/program.o: program.cpp program.h $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ program.cpp

$(BIN)/file_parser.o: file_parser.cpp file_parser.h $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ file_parser.cpp

$(BIN)/file_writer.o: file_writer.cpp file_writer.h $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ file_writer.cpp

lgrp.cc: LGRParser.l
	flexc++ LGRParser.l -l lgrp.cc

$(BIN)/lex.o: $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ lgrp.cc

$(BIN)/LGRParser.o: LGRParser.cpp LGRParser.h $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ LGRParser.cpp

$(BIN)/bootstrap_segments_generator.o: bootstrap_segments_generator.cpp bootstrap_segments_generator.h $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ bootstrap_segments_generator.cpp

bootstrap_segments_generator.out: $(BIN)/bootstrap_segments_generator.o $(shared_depenedencies)
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/bootstrap_segments_generator.o $(shared_depenedencies)

$(BIN)/bootstrap_set_selector.o: bootstrap_set_selector.cpp bootstrap_set_selector.h $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ bootstrap_set_selector.cpp

bootstrap_set_selector.out: $(BIN)/bootstrap_set_selector.o $(shared_depenedencies)
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/bootstrap_set_selector.o $(shared_depenedencies)

$(BIN)/list_scheduler.o: list_scheduler.cpp list_scheduler.h $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ list_scheduler.cpp

list_scheduler.out: $(BIN)/list_scheduler.o $(shared_depenedencies)
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/list_scheduler.o $(shared_depenedencies)

$(BIN)/complete_to_selective_converter.o: complete_to_selective_converter.cpp $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ complete_to_selective_converter.cpp

complete_to_selective_converter.out: $(BIN)/complete_to_selective_converter.o $(shared_depenedencies)
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/complete_to_selective_converter.o $(shared_depenedencies)

$(BIN)/ldt_generator.o: ldt_generator.cpp $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ ldt_generator.cpp

ldt_generator.out: $(BIN)/ldt_generator.o $(shared_depenedencies)
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/ldt_generator.o $(shared_depenedencies)

$(BIN)/random_graph_generator.o: random_graph_generator.cpp random_graph_generator.h $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ random_graph_generator.cpp

random_graph_generator.out: $(BIN)/random_graph_generator.o $(BIN)/shared_utils.o $(primitive_dependencies)
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/random_graph_generator.o $(BIN)/shared_utils.o $(primitive_dependencies)

$(BIN)/txt_to_vcg.o: txt_to_vcg.cpp $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ txt_to_vcg.cpp

txt_to_vcg.out: $(BIN)/txt_to_vcg.o $(BIN)/shared_utils.o $(primitive_dependencies)
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/txt_to_vcg.o $(BIN)/shared_utils.o $(primitive_dependencies)
	
$(BIN)/lgr_to_sched.o: lgr_to_sched.cpp $(o_dependencies)
	$(CXX) $(CPP_FLAGS) -c -o $@ lgr_to_sched.cpp

lgr_to_sched.out: $(BIN)/lgr_to_sched.o $(BIN)/shared_utils.o $(primitive_dependencies)
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/lgr_to_sched.o $(BIN)/shared_utils.o $(primitive_dependencies)

clean:
	rm lex.cc lgrp.cc LGRParserBase.h lex.yy.c *.out bin/*