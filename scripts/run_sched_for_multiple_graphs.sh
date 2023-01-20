#!/bin/bash

#Usage <script_name> <first_graph_num> <last_graph_num> <heuristic_name> <sched_name> <num_cores> <num_levels> <rand_thresh> <boot_mode>

first_graph_num=$1
last_graph_num=$2
heuristic_name=$3
sched_name=$4
num_cores=$5
num_levels=$6
rand_thresh=$7
boot_mode=$8

make -C ./CPP_code/execution_engine/build

# heuristic=("1 0 0" "1 0 100" "2 1 0" "2 1 100")

for i in $(seq $first_graph_num $last_graph_num)
do
    sched_file=./results/random_graph$i/${num_levels}_levels/${heuristic_name}/${num_cores}_cores/rg${i}_${sched_name}
    
    echo ./CPP_code/execution_engine/build/execution_engine $sched_file $num_cores $num_levels $rand_thresh $boot_mode
    ./CPP_code/execution_engine/build/execution_engine $sched_file $num_cores $num_levels $rand_thresh $boot_mode
done
