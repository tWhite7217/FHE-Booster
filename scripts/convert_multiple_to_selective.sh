#!/bin/bash

make limited_to_selective_converter.out

#Usage <script_name> <input_lgr_name> <input_lgr_folder> <output_lgr_name> <first_graph_num> <last_graph_num> <num_levels>

input_lgr_name=$1
input_lgr_folder=$2
output_lgr_name=$3
first_graph_num=$4
last_graph_num=$5
num_levels=$6

for i in $(seq $first_graph_num $last_graph_num)
do
    
    dag_file="DAGs/random_graph${i}/random_graph${i}.txt"
    input_lgr="results/random_graph$i/${num_levels}_levels/$input_lgr_folder/rg${i}_${input_lgr_name}.lgr"
    output_lgr="results/random_graph$i/${num_levels}_levels/bootstrap_sets/rg${i}_${output_lgr_name}_converted.lgr"
    
    echo ./CPP_code/limited_to_selective_converter.out $dag_file $input_lgr $output_lgr $num_levels
    ./CPP_code/limited_to_selective_converter.out $dag_file $input_lgr $output_lgr $num_levels
    
done