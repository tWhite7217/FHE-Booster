#!/bin/bash

first_graph_num=$1
last_graph_num=$2
num_cores=$3
bootstrap_levels=$4
initial_levels=$5
first_save_num=$6
last_save_num=$7

for j in $(seq $first_save_num $last_save_num)
do
    ./scripts/run_sched_for_multiple_graphs.sh $first_graph_num $last_graph_num no_bootstrapping no_bootstrapping $num_cores $bootstrap_levels $initial_levels "-s -o $j -m ALAP"
    # ./scripts/run_sched_for_multiple_graphs.sh $first_graph_num $last_graph_num min_bootstrapping complete $num_cores $bootstrap_levels $initial_levels "-s -o $j -m BOOSTER"
    ./scripts/run_sched_for_multiple_graphs.sh $first_graph_num $last_graph_num min_bootstrapping converted $num_cores $bootstrap_levels $initial_levels "-s -o $j -m BOOSTER"
    
    heuristic_nums=("1 0 0" "0 1 0" "0 0 1" "1 1 0" "1 0 1" "0 1 1" "1 1 1")
    
    for ((k = 0; k < ${#heuristic_nums[@]}; k++))
    do
        IFS=' ' read -ra vals <<< "${heuristic_nums[k]}"
        heuristic_name="s${vals[0]}_r${vals[1]}_u${vals[2]}"
    
        # ./scripts/run_sched_for_multiple_graphs.sh $first_graph_num $last_graph_num $heuristic_name complete $num_cores $bootstrap_levels $initial_levels "-s -o $j -m BOOSTER"
        ./scripts/run_sched_for_multiple_graphs.sh $first_graph_num $last_graph_num $heuristic_name converted $num_cores $bootstrap_levels $initial_levels "-s -o $j -m BOOSTER"
    done
done
