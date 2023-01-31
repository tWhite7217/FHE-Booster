#!/bin/bash

#Usage <script_name> <first_graph_num> <last_graph_num> <num_cores> <num_levels>

first_graph_num=$1
last_graph_num=$2
num_cores=$3
num_levels=$4


# ./scripts/list_schedule_multiple.sh min_bootstrapping.lgr $first_graph_num $last_graph_num min_bootstrapping limited $num_levels $num_cores
# ./scripts/convert_multiple_to_selective.sh min_bootstrapping bootstrapping_sets min_bootstrapping $first_graph_num $last_graph_num $num_levels
# ./scripts/list_schedule_multiple.sh min_bootstrapping_converted.lgr $first_graph_num $last_graph_num min_bootstrapping selective $num_levels $num_cores


# heuristic_nums=("1 0 0" "0 1 0" "0 0 1" "1 1 0" "1 0 1" "0 1 1" "1 1 1")
heuristic_nums=("1 1 1")

for ((i = 0; i < ${#heuristic_nums[@]}; i++))
do
    echo "$i"
    IFS=' ' read -ra vals <<< "${heuristic_nums[i]}"
    heuristic_name="s${vals[0]}_r${vals[1]}_u${vals[2]}"
    heuristic_options="-s ${vals[0]} -r ${vals[1]} -u ${vals[2]}"
    echo "$heuristic_name"
    echo "$heuristic_options"

    ./scripts/list_schedule_multiple.sh NULL $first_graph_num $last_graph_num $heuristic_name limited $num_levels $num_cores "$heuristic_options"
    ./scripts/convert_multiple_to_selective.sh limited $heuristic_name/${num_cores}_cores $heuristic_name $first_graph_num $last_graph_num $num_levels
    ./scripts/list_schedule_multiple.sh ${heuristic_name}_converted.lgr $first_graph_num $last_graph_num $heuristic_name selective $num_levels $num_cores
done