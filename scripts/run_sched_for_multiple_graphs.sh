#!/bin/bash

#Usage <script_name> <first_graph_num> <last_graph_num> <heuristic_name> <sched_name> <num_cores> <bootstrap_levels> <initial_levels> <engine_options>

first_graph_num=$1
last_graph_num=$2
heuristic_name=$3
sched_name=$4
num_cores=$5
bootstrap_levels=$6
initial_levels=$7
inputs=$8
engine_options=$9

make -C ./CPP_code/execution_engine/build

for i in $(seq $first_graph_num $last_graph_num)
do
    inputs_argument=""
    if [[ "$inputs" = "Y" ]]; then
        echo "here"
        inputs_filename="./DAGs/random_graph${i}/inputs.txt"
        inputs_argument="-i FILE,${inputs_filename} -v"
    fi
    
    sched_file=./results/random_graph$i/b${bootstrap_levels}_i${initial_levels}/${heuristic_name}/${num_cores}_cores/rg${i}_${sched_name}
    
    echo ./CPP_code/execution_engine/build/execution_engine $sched_file -l $bootstrap_levels $inputs_argument $engine_options
    ./CPP_code/execution_engine/build/execution_engine $sched_file -l $bootstrap_levels $inputs_argument $engine_options
done
