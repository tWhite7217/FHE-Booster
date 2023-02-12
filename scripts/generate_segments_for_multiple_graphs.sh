#!/bin/bash

#Usage: <script_name> <first_graph_num> <last_graph_num> <num_levels> <initial_levels>

make bootstrap_segments_generator.out

first_graph_num=$1
last_graph_num=$2
num_levels=$3
initial_levels=$4

for i in $(seq $first_graph_num $last_graph_num)
do
    dag_file="DAGs/random_graph$i/random_graph$i.txt"
    output_file="DAGs/random_graph$i/${num_levels}_levels/bootstrap_segments"
    
    echo ./CPP_code/bootstrap_segments_generator.out $dag_file $output_file $num_levels -i $initial_levels
    ./CPP_code/bootstrap_segments_generator.out $dag_file $output_file $num_levels -i $initial_levels
done