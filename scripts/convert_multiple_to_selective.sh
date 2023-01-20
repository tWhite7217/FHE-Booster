#!/bin/bash

make limited_to_selective_converter.out

#Usage <script_name> <lgr_name> <lgr_folder> <first_graph_num> <last_graph_num> <num_levels>

lgr_name=$1
lgr_folder=$2
first_graph_num=$3
last_graph_num=$4
num_levels=$5

for i in $(seq $first_graph_num $last_graph_num)
do

    dag_file="DAGs/random_graph${i}/random_graph${i}.txt"
    input_lgr="results/random_graph$i/${num_levels}_levels/$lgr_folder/rg${i}_${lgr_name}.lgr"
    output_lgr="results/random_graph$i/${num_levels}_levels/bootstrapping_sets/rg${i}_${lgr_name}_converted.lgr"

    echo ./CPP_code/limited_to_selective_converter.out $dag_file $input_lgr $output_lgr $num_levels
    ./CPP_code/limited_to_selective_converter.out $dag_file $input_lgr $output_lgr $num_levels
    
done