#!/bin/bash

#Usage <script_name> <first_graph_num> <last_graph_num> <num_cores> <levels>

first_graph_num=$1
last_graph_num=$2
num_cores=$3
levels=$4

./scripts/list_schedule_multiple.sh $first_graph_num $last_graph_num NULL no_bootstrapping $levels $num_cores
# ^ Briefly thought this should be handled separately, since this is not dependent
# on levels. However, levels will impact the eval time when this sched is actually
# run, so for simplicity sake, I will leave this.

./scripts/list_schedule_multiple.sh $first_graph_num $last_graph_num min_bootstrapping complete $levels $num_cores
./scripts/list_schedule_multiple.sh $first_graph_num $last_graph_num min_bootstrapping converted $levels $num_cores

heuristic_nums=("1 0 0" "0 1 0" "0 0 1" "1 1 0" "1 0 1" "0 1 1" "1 1 1" "2 0 1" "2 1 0" "2 1 1" "4 0 1" "4 1 0" "4 1 1" "4 1 2" "4 2 1")

for ((i = 0; i < ${#heuristic_nums[@]}; i++))
do
    IFS=' ' read -ra vals <<< "${heuristic_nums[i]}"
    # heuristic_name="modified_s${vals[0]}_r${vals[1]}_u${vals[2]}"
    heuristic_name="s${vals[0]}_r${vals[1]}_u${vals[2]}"

    ./scripts/list_schedule_multiple.sh $first_graph_num $last_graph_num $heuristic_name complete $levels $num_cores
    ./scripts/list_schedule_multiple.sh $first_graph_num $last_graph_num $heuristic_name converted $levels $num_cores
done