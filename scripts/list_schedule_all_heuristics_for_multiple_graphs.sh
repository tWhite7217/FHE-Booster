#!/bin/bash

#Usage <script_name> <first_graph_num> <last_graph_num> <num_cores> <levels>

first_graph_num=$1
last_graph_num=$2
num_cores=$4
levels=$4

# ./scripts/list_schedule_multiple.sh $first_graph_num $last_graph_num NULL no_bootstrapping $levels $num_cores

./scripts/list_schedule_multiple.sh $first_graph_num $last_graph_num min_bootstrapping complete $levels $num_cores
./scripts/list_schedule_multiple.sh $first_graph_num $last_graph_num min_bootstrapping converted $levels $num_cores

heuristic_nums=("1 0 0" "0 1 0" "0 0 1" "1 1 0" "1 0 1" "0 1 1" "1 1 1")
# heuristic_nums=("1 1 1")

for ((i = 0; i < ${#heuristic_nums[@]}; i++))
do
    echo "$i"
    IFS=' ' read -ra vals <<< "${heuristic_nums[i]}"
    heuristic_name="s${vals[0]}_r${vals[1]}_u${vals[2]}"
    heuristic_options="-s ${vals[0]} -r ${vals[1]} -u ${vals[2]}"
    echo "$heuristic_name"
    echo "$heuristic_options"

    ./scripts/list_schedule_multiple.sh $first_graph_num $last_graph_num $heuristic_name limited $levels $num_cores
    ./scripts/list_schedule_multiple.sh $first_graph_num $last_graph_num $heuristic_name converted $levels $num_cores
done