#!/bin/bash

first_graph_num=$1
last_graph_num=$2
num_cores=$3
bootstrap_levels=$4
initial_levels=$5
first_save_num=$6
last_save_num=$7
inputs=$8


for i in $(seq $first_graph_num $last_graph_num)
do
    for j in $(seq $first_save_num $last_save_num)
    do
        ./scripts/run_sched_for_multiple_graphs.sh $i $i no_bootstrapping no_bootstrapping $num_cores $bootstrap_levels $initial_levels $inputs "-s -o $j -m ALAP c"
        # ./scripts/run_sched_for_multiple_graphs.sh $i $i min_bootstrapping complete $num_cores $bootstrap_levels $initial_levels $inputs "-s -o $j -m BOOSTER"
        ./scripts/run_sched_for_multiple_graphs.sh $i $i min_bootstrapping converted $num_cores $bootstrap_levels $initial_levels $inputs "-s -o $j -m BOOSTER"
        
        heuristic_nums=("1 0 0" "0 1 0" "0 0 1" "1 1 0" "1 0 1" "0 1 1" "1 1 1" "2 0 1" "2 1 0" "2 1 1" "4 0 1" "4 1 0" "4 1 1" "4 1 2" "4 2 1")
        
        for ((k = 0; k < ${#heuristic_nums[@]}; k++))
        do
            IFS=' ' read -ra vals <<< "${heuristic_nums[k]}"
            # heuristic_name="modified_s${vals[0]}_r${vals[1]}_u${vals[2]}"
            heuristic_name="s${vals[0]}_r${vals[1]}_u${vals[2]}"
        
            # ./scripts/run_sched_for_multiple_graphs.sh $i $i $heuristic_name complete $num_cores $bootstrap_levels $initial_levels $inputs "-s -o $j -m BOOSTER"
            ./scripts/run_sched_for_multiple_graphs.sh $i $i $heuristic_name converted $num_cores $bootstrap_levels $initial_levels $inputs "-s -o $j -m BOOSTER"
        done
    done
done
