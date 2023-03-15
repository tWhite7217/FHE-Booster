#!/bin/bash

# Usage: <script_name> <first_graph_num> <last_graph_num> <heuristic_name> <levels> <num_cores> <sched_name> <boot_mode> <output_num>

graph_num=$1
levels=$2
num_cores=$3
first_save_num=$5
last_save_num=$6

heuristic_nums=("1 0 0" "0 1 0" "0 0 1" "1 1 0" "1 0 1" "0 1 1" "1 1 1")

heuristic_names=("no_bootstrapping" "min_bootstrapping" "min_bootstrapping")
sched_names=("no_bootstrapping" "complete" "converted")
boot_modes=("ALAP" "BOOSTER" "BOOSTER")

for ((k = 0; k < ${#heuristic_nums[@]}; k++))
do
    IFS=' ' read -ra vals <<< "${heuristic_nums[k]}"
    heuristic_names+=("s${vals[0]}_r${vals[1]}_u${vals[2]}")
    sched_names+=("converted")
    boot_modes+=("BOOSTER")
done

for ((k = 0; k < ${#heuristic_names[@]}; k++))
do
    heuristic_name="${heuristic_names[k]}"
    sched_name="${sched_names[k]}"
    boot_mode="${boot_modes[k]}"
    echo "$heuristic_name,$sched_name,$boot_mode:"
    for i in $(seq $first_save_num $last_save_num)
    do
        cat ./results/random_graph${graph_num}/${levels}/${heuristic_name}/${num_cores}_cores/rg${graph_num}_${sched_name}_eval_time_${boot_mode}_${i}.txt
    done
    echo
done