#!/bin/bash

#Usage: <script_name> <first_graph_num> <last_graph_num> <bootstrap_levels> <initial_levels>

make bootstrap_segments_generator.out

first_graph_num=$1
last_graph_num=$2
bootstrap_levels=$3
initial_levels=$4

for i in $(seq $first_graph_num $last_graph_num)
do
    dag_file="DAGs/random_graph$i/random_graph$i.txt"
    output_dir="DAGs/random_graph$i/b${bootstrap_levels}_i${initial_levels}"
    
    mkdir $output_dir
    
    output_file="${output_dir}/bootstrap_segments"
    
    echo ./CPP_code/bootstrap_segments_generator.out $dag_file $output_file $bootstrap_levels -i $initial_levels
    ./CPP_code/bootstrap_segments_generator.out $dag_file $output_file $bootstrap_levels -i $initial_levels
done