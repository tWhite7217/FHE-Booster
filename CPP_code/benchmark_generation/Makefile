CXX = g++-10

BIN = ./bin

CPP_FLAGS = -std=c++20 -O3

all: make_conv_dag.out make_lr_dag.out make_pendulum_dag.out make_pid_dag.out

$(BIN)/dag_generator.o: dag_generator.h dag_generator.cpp Makefile
	$(CXX) $(CPP_FLAGS) -c -o $@ dag_generator.cpp
	
$(BIN)/make_nn_dag.o: make_nn_dag.cpp Makefile
	$(CXX) $(CPP_FLAGS) -c -o $@ make_nn_dag.cpp

$(BIN)/make_simon_dag.o: make_simon_dag.cpp Makefile
	$(CXX) $(CPP_FLAGS) -c -o $@ make_simon_dag.cpp

$(BIN)/make_lr_dag.o: make_lr_dag.cpp Makefile
	$(CXX) $(CPP_FLAGS) -c -o $@ make_lr_dag.cpp

$(BIN)/make_conv_dag.o: make_conv_dag.cpp Makefile
	$(CXX) $(CPP_FLAGS) -c -o $@ make_conv_dag.cpp

$(BIN)/make_conv_variable_dag.o: make_conv_variable_dag.cpp Makefile
	$(CXX) $(CPP_FLAGS) -c -o $@ make_conv_variable_dag.cpp

$(BIN)/make_pid_dag.o: make_pid_dag.cpp Makefile
	$(CXX) $(CPP_FLAGS) -c -o $@ make_pid_dag.cpp

$(BIN)/make_pendulum_dag.o: make_pendulum_dag.cpp Makefile
	$(CXX) $(CPP_FLAGS) -c -o $@ make_pendulum_dag.cpp

make_nn_dag.out: $(BIN)/make_nn_dag.o $(BIN)/dag_generator.o
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/make_nn_dag.o $(BIN)/dag_generator.o

make_simon_dag.out: $(BIN)/make_simon_dag.o $(BIN)/dag_generator.o
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/make_simon_dag.o $(BIN)/dag_generator.o

make_lr_dag.out: $(BIN)/make_lr_dag.o $(BIN)/dag_generator.o
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/make_lr_dag.o $(BIN)/dag_generator.o

make_conv_dag.out: $(BIN)/make_conv_dag.o $(BIN)/dag_generator.o
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/make_conv_dag.o $(BIN)/dag_generator.o

make_conv_variable_dag.out: $(BIN)/make_conv_variable_dag.o $(BIN)/dag_generator.o
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/make_conv_variable_dag.o $(BIN)/dag_generator.o

make_pid_dag.out: $(BIN)/make_pid_dag.o $(BIN)/dag_generator.o
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/make_pid_dag.o $(BIN)/dag_generator.o

make_pendulum_dag.out: $(BIN)/make_pendulum_dag.o $(BIN)/dag_generator.o
	$(CXX) $(CPP_FLAGS) -o $@ $(BIN)/make_pendulum_dag.o $(BIN)/dag_generator.o

clean:
	rm make_nn_dag txt_to_vcg
