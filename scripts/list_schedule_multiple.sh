#!/bin/bash

#Usage <script_name> <source_lgr> <first_graph_num> <last_graph_num> <heuristic_name> <bootstrap_mode> <num_levels> <num_cores>

make list_scheduler.out

first_graph_num=$1
last_graph_num=$2
heuristic_name=$3
bootstrap_mode=$4
num_levels=$5
num_cores=$6

for i in $(seq $first_graph_num $last_graph_num)
do
    dag_file="DAGs/random_graph$i/random_graph$i.txt"
    
    bootstrap_set_file="NULL"
    if [[ "$heuristic_name" != "NULL" ]]; then
        bootstrap_set_file="results/random_graph$i/${num_levels}_levels/$heuristic_name/${bootstrap_mode}_bootstrap_set.lgr"
    else
        heuristic_name="no_bootstrapping"
    fi
    
    result_dir="results/random_graph$i"
    mkdir $result_dir
    result_dir="$result_dir/${num_levels}_levels"
    mkdir $result_dir
    result_dir="$result_dir/$heuristic_name"
    mkdir $result_dir
    result_dir="$result_dir/${num_cores}_cores"
    mkdir $result_dir
    result_file="$result_dir/rg${i}_${bootstrap_mode}"
    
    echo ./CPP_code/list_scheduler.out $dag_file $result_file -i $bootstrap_set_file -t $num_cores
    ./CPP_code/list_scheduler.out $dag_file $result_file -i $bootstrap_set_file -t $num_cores
    
done