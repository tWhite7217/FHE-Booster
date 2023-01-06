#!/bin/bash

./run_lingo_solver_on_multiple_graphs.sh min_bootstrapping $1 $2
./run_lingo_solver_on_multiple_graphs.sh min_bootstrapping_selective $1 $2
./run_lingo_solver_on_multiple_graphs.sh unlimited $1 $2
./run_lingo_solver_on_multiple_graphs.sh limited $1 $2 1
./run_lingo_solver_on_multiple_graphs.sh limited $1 $2 2
# ./run_lingo_solver_on_multiple_graphs.sh selective $1 $2 1
# ./run_lingo_solver_on_multiple_graphs.sh selective $1 $2 2