#!/bin/bash

# Usage: <script_name> <first_graph_num> <last_graph_num> <heuristic_name> <levels> <num_cores> <sched_name> <boot_mode>

first_graph_num=$1
last_graph_num=$2
heuristic_name=$3
levels=$4
num_cores=$5
sched_name=$6
boot_mode=$7
first_output_num=$8
last_output_num=$9

for i in $(seq $first_graph_num $last_graph_num)
do
    # mod_time=$(stat -c %Y ./results/random_graph$i/${levels}/${heuristic_name}/${num_cores}_cores/rg${i}_${sched_name}_eval_time_${boot_mode}_${output_num}.txt)
    # if [ $mod_time -lt 1680202500 ]; then
    #     echo "${heuristic_name} rg${i}"
    # fi
    for j in $(seq $first_output_num $last_output_num)
    do
        cat ./results/random_graph$i/${levels}/${heuristic_name}/${num_cores}_cores/rg${i}_${sched_name}_eval_time_${boot_mode}_${j}.txt
    done
done

echo

for i in $(seq $first_graph_num $last_graph_num)
do
    
    cat ./results/random_graph$i/${levels}/${heuristic_name}/${num_cores}_cores/rg${i}_${sched_name}_num_bootstraps_${boot_mode}.txt
    
done