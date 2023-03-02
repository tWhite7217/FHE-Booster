#!/bin/bash

#Usage <script_name> <first_graph_num> <last_graph_num> <levels> <segments_weight> <slack_weight> <urgency_weight>

make bootstrap_set_selector.out

first_graph_num=$1
last_graph_num=$2
levels=$3
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
    segments_file="DAGs/random_graph$i/$levels/bootstrap_segments_standard.dat"

    output_file_base="results/random_graph$i"
    mkdir $output_file_base
    output_file_base="$output_file_base/$levels"
    mkdir $output_file_base

    output_files=""

    for ((j = 0; j < ${#segments_weights[@]}; j++))
    do
        heuristic_name="s${segments_weights[j]}_r${slack_weights[j]}_u${urgency_weights[j]}"
        output_file_dir="$output_file_base/$heuristic_name"
        mkdir $output_file_dir
        output_files="${output_files}${output_file_dir}/complete_bootstrap_set,"
    done

    output_files=${output_files::-1}

    echo ./CPP_code/bootstrap_set_selector.out $dag_file $segments_file $output_files -s $segments_weight -r $slack_weight -u $urgency_weight
    ./CPP_code/bootstrap_set_selector.out $dag_file $segments_file $output_files -s $segments_weight -r $slack_weight -u $urgency_weight

done