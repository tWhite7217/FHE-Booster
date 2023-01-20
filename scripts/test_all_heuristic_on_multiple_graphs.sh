#!/bin/bash

#Usage <script_name> <first_graph_num> <last_graph_num> <num_cores> <num_levels>

first_graph_num=$1
last_graph_num=$2
num_cores=$3
num_levels=$4


./scripts/list_schedule_multiple.sh min_bootstrapping.lgr $first_graph_num $last_graph_num $num_cores $num_levels min_bootstrapping 0 0 0 limited
./scripts/convert_multiple_to_selective.sh min_bootstrapping bootstrapping_sets $first_graph_num $last_graph_num $num_levels
./scripts/list_schedule_multiple.sh min_bootstrapping_converted.lgr $first_graph_num $last_graph_num $num_cores $num_levels min_bootstrapping 0 0 0 selective


heuristic_nums=("1 0 0" "1 0 100" "2 1 0" "2 1 100")

for ((i = 0; i < ${#heuristic_nums[@]}; i++))
do
    echo "$i"
    IFS=' ' read -ra vals <<< "${heuristic_nums[i]}"
    heuristic_name="pm${vals[0]}_sm${vals[1]}_um${vals[2]}"
    echo "$heuristic_name"
    
    ./scripts/list_schedule_multiple.sh NULL $first_graph_num $last_graph_num $num_cores $num_levels $heuristic_name ${heuristic_nums[i]} limited
    ./scripts/convert_multiple_to_selective.sh limited $heuristic_name/${num_cores}_cores $first_graph_num $last_graph_num $num_levels
    ./scripts/list_schedule_multiple.sh ${heuristic_name}_converted.lgr $first_graph_num $last_graph_num $num_cores $num_levels $heuristic_name 0 0 0 selective
done
