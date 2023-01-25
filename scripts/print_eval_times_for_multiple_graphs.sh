#!/bin/bash

# Usage: <script_name> <first_graph_num> <last_graph_num> <heuristic_name> <num_levels> <num_cores> <sched_name> <boot_mode> <output_num>

first_graph_num=$1
last_graph_num=$2
heuristic_name=$3
num_levels=$4
num_cores=$5
sched_name=$6
boot_mode=$7
output_num=$8

for i in $(seq $first_graph_num $last_graph_num)
do
    
    cat ./results/random_graph$i/${num_levels}_levels/${heuristic_name}/${num_cores}_cores/rg${i}_${sched_name}_eval_time_${boot_mode}_${output_num}.txt
    
done