#!/bin/bash

#Usage <script_name> <first_graph_num> <last_graph_num> <num_levels> <segments_weight> <slack_weight> <urgency_weight>

make bootstrap_set_selector.out

first_graph_num=$1
last_graph_num=$2
num_levels=$3
segments_weight=$4
slack_weight=$5
urgency_weight=$6

IFS=',' read -ra segments_weights <<< "$segments_weight"
IFS=',' read -ra slack_weights <<< "$slack_weight"
IFS=',' read -ra urgency_weights <<< "$urgency_weight"

echo ${heuristic_names[1]}

for i in $(seq $first_graph_num $last_graph_num)
do
    dag_file="DAGs/random_graph$i/random_graph$i.txt"
    segments_file="DAGs/random_graph$i/${num_levels}_levels/bootstrap_segments.txt"

    output_file_base="results/random_graph$i"
    mkdir $output_file_base
    output_file_base="$output_file_base/${num_levels}_levels"
    mkdir $output_file_base
    output_file_base="$output_file_base/bootstrapping_sets"
    mkdir $output_file_base

    output_files=""

    for ((j = 0; j < ${#segments_weights[@]}; j++))
    do
        output_file_main="rg${i}_s${segments_weights[j]}_r${slack_weights[j]}_u${urgency_weights[j]}"
        output_files="${output_files}${output_file_base}/${output_file_main},"
    done

    output_files=${output_files::-1}

    echo ./CPP_code/bootstrap_set_selector.out $dag_file $segments_file $output_files $num_levels -s $segments_weight -r $slack_weight -u $urgency_weight
    ./CPP_code/bootstrap_set_selector.out $dag_file $segments_file $output_files $num_levels -s $segments_weight -r $slack_weight -u $urgency_weight

done