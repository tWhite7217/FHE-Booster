#!/bin/bash

# Usage: <script_name> <first_graph_num> <last_graph_num> <heuristic_name> <levels> <num_cores> <sched_name> <boot_mode>

first_graph_num=$1
last_graph_num=$2
levels=$3
num_cores=$4

# heuristic_nums=("1 0 0" "0 1 0" "0 0 1" "1 1 0" "1 0 1" "0 1 1" "1 1 1" "2 0 1" "2 1 0" "2 1 1" "4 0 1" "4 1 0" "4 1 1" "4 1 2" "4 2 1")
heuristic_nums=("1 0 0" "0 1 0" "0 0 1" "1 1 0" "1 0 1" "0 1 1" "1 1 1")

# heuristic_names=("min_bootstrapping" "min_bootstrapping")
# sched_names=("complete" "converted")
# boot_modes=("BOOSTER" "BOOSTER")

for ((k = 0; k < ${#heuristic_nums[@]}; k++))
do
    IFS=' ' read -ra vals <<< "${heuristic_nums[k]}"
    heuristic_names+=("modified_s${vals[0]}_r${vals[1]}_u${vals[2]}")
    # heuristic_names+=("s${vals[0]}_r${vals[1]}_u${vals[2]}")
    sched_names+=("converted")
    boot_modes+=("BOOSTER")
done

for i in $(seq $first_graph_num $last_graph_num)
do
    for ((k = 0; k < ${#heuristic_names[@]}; k++))
    do
        heuristic_name="${heuristic_names[k]}"
        sched_name="${sched_names[k]}"
        boot_mode="${boot_modes[k]}"

        grep -a "Objective value" ./results/random_graph$i/${levels}/${heuristic_name}/${num_cores}_cores/rg${i}_${sched_name}.lgr | sed 's/\./ /g' | awk '{print $3}'
        # cat ./results/random_graph$i/${levels}/${heuristic_name}/${num_cores}_cores/rg${i}_${sched_name}_eval_time_${boot_mode}_${j}.txt
    done
done

echo