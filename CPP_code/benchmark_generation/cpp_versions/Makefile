CXX = g++-10

# BIN = ./bin

# primitive_dependencies = $(BIN)/operation_type.o $(BIN)/operation.o $(BIN)/bootstrap_segment.o $(BIN)/program.o $(BIN)/file_parser.o $(BIN)/lex.o $(BIN)/LGRParser.o $(BIN)/file_writer.o

# shared_depenedencies = $(BIN)/shared_utils.o $(primitive_dependencies)

# o_dependencies = lgrp.cc Makefile

CPP_FLAGS = -std=c++20 -O3

# all: bootstrap_segments_generator.out bootstrap_set_selector.out list_scheduler.out complete_to_selective_converter.out random_graph_generator.out ldt_generator.out

conv.out: conv.cpp
	$(CXX) $(CPP_FLAGS) -o $@ conv.cpp

conv_variable.out: conv_variable.cpp
	$(CXX) $(CPP_FLAGS) -o $@ conv_variable.cpp

pid.out: pid.cpp
	$(CXX) $(CPP_FLAGS) -o $@ pid.cpp

pendulum.out: pendulum.cpp
	$(CXX) $(CPP_FLAGS) -o $@ pendulum.cpp

sine_newton.out: sine_newton.cpp
	$(CXX) $(CPP_FLAGS) -o $@ sine_newton.cpp

clean:
	rm conv.out conv_variable.out