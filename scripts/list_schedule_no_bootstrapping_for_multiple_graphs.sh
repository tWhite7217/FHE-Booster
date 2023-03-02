#!/bin/bash

#Usage <script_name> <source_lgr> <first_graph_num> <last_graph_num> <num_cores>

make list_scheduler.out

first_graph_num=$1
last_graph_num=$2
num_cores=$3

for i in $(seq $first_graph_num $last_graph_num)
do
    dag_file="DAGs/random_graph$i/random_graph$i.txt"
    
    result_dir="results/random_graph$i"
    mkdir $result_dir
    result_dir="$result_dir/no_bootstrapping"
    mkdir $result_dir
    result_dir="$result_dir/${num_cores}_cores"
    mkdir $result_dir
    result_file="$result_dir/rg${i}_no_bootstrapping"
    
    echo ./CPP_code/list_scheduler.out $dag_file $result_file -t $num_cores
    ./CPP_code/list_scheduler.out $dag_file $result_file -t $num_cores
    
done