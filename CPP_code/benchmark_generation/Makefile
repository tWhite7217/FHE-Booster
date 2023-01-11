BIN = ./bin

CPP_FLAGS = -std=c++17 -O3

all: make_nn_dag.out make_simon_dag.out make_lr_dag.out

$(BIN)/dag_generator.o: dag_generator.h dag_generator.cpp Makefile
	g++ $(CPP_FLAGS) -c -o $@ dag_generator.cpp
	
$(BIN)/make_nn_dag.o: make_nn_dag.cpp Makefile
	g++ $(CPP_FLAGS) -c -o $@ make_nn_dag.cpp

$(BIN)/make_simon_dag.o: make_simon_dag.cpp Makefile
	g++ $(CPP_FLAGS) -c -o $@ make_simon_dag.cpp

$(BIN)/make_lr_dag.o: make_lr_dag.cpp Makefile
	g++ $(CPP_FLAGS) -c -o $@ make_lr_dag.cpp

make_nn_dag.out: $(BIN)/make_nn_dag.o $(BIN)/dag_generator.o
	g++ $(CPP_FLAGS) -o $@ $(BIN)/make_nn_dag.o $(BIN)/dag_generator.o

make_simon_dag.out: $(BIN)/make_simon_dag.o $(BIN)/dag_generator.o
	g++ $(CPP_FLAGS) -o $@ $(BIN)/make_simon_dag.o $(BIN)/dag_generator.o

make_lr_dag.out: $(BIN)/make_lr_dag.o $(BIN)/dag_generator.o
	g++ $(CPP_FLAGS) -o $@ $(BIN)/make_lr_dag.o $(BIN)/dag_generator.o

clean:
	rm make_nn_dag txt_to_vcg