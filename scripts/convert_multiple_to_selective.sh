#!/bin/bash

make complete_to_selective_converter.out

#Usage <script_name> <first_graph_num> <last_graph_num> <heuristic_name> <num_levels>

first_graph_num=$1
last_graph_num=$2
heuristic_name=$3
num_levels=$4

for i in $(seq $first_graph_num $last_graph_num)
do
    
    dag_file="DAGs/random_graph${i}/random_graph${i}.txt"
    segments_file="DAGs/random_graph${i}/${num_levels}_levels/bootstrap_segments_selective.txt"
    input_bootstrap_set="results/random_graph$i/${num_levels}_levels/$heuristic_name/complete_bootstrap_set.lgr"
    output_bootstrap_set="results/random_graph$i/${num_levels}_levels/$heuristic_name/converted_bootstrap_set.lgr"
    
    echo ./CPP_code/complete_to_selective_converter.out $dag_file $segments_file $input_bootstrap_set $output_bootstrap_set
    ./CPP_code/complete_to_selective_converter.out $dag_file $segments_file $input_bootstrap_set $output_bootstrap_set
    
done