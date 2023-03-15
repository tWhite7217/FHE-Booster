#!/bin/bash

#Usage <script_name> <first_graph_num> <last_graph_num> <levels>

first_graph_num=$1
last_graph_num=$2
levels=$3

./scripts/convert_multiple_to_selective.sh $first_graph_num $last_graph_num min_bootstrapping $levels

heuristic_nums=("1 0 0" "0 1 0" "0 0 1" "1 1 0" "1 0 1" "0 1 1" "1 1 1")

segments_weight=""
slack_weight=""
urgency_weight=""

heuristic_names=()

for ((i = 0; i < ${#heuristic_nums[@]}; i++))
do
    # echo "$i"
    IFS=' ' read -ra vals <<< "${heuristic_nums[i]}"

    segments_weight="${segments_weight}${vals[0]},"
    slack_weight="${slack_weight}${vals[1]},"
    urgency_weight="${urgency_weight}${vals[2]},"
done

segments_weight=${segments_weight::-1}
slack_weight=${slack_weight::-1}
urgency_weight=${urgency_weight::-1}

./scripts/generate_bootstrap_sets_for_multiple_graphs.sh $first_graph_num $last_graph_num $levels $segments_weight $slack_weight $urgency_weight

for ((i = 0; i < ${#heuristic_nums[@]}; i++))
do
    IFS=' ' read -ra vals <<< "${heuristic_nums[i]}"
    heuristic_name="s${vals[0]}_r${vals[1]}_u${vals[2]}"
    ./scripts/convert_multiple_to_selective.sh $first_graph_num $last_graph_num $heuristic_name $levels
done