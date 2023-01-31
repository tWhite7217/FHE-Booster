#!/bin/bash

#Usage <script_name> <source_lgr> <first_graph_num> <last_graph_num> <heuristic_name> <sched_name> <num_levels> <num_cores> <options>

make list_scheduler.out

source_lgr=$1
first_graph_num=$2
last_graph_num=$3
heuristic_name=$4
sched_name=$5
num_levels=$6
num_cores=$7
options=$8

source_lgr_base=""

for i in $(seq $first_graph_num $last_graph_num)
do
    
    dag_file="DAGs/random_graph$i/random_graph$i.txt"
    
    if [[ "$source_lgr" != "NULL" ]]; then
        source_lgr_base="-i results/random_graph$i/${num_levels}_levels/bootstrapping_sets/rg${i}_"
    else
        source_lgr=""
    fi
    
    result_folder="results/random_graph$i"
    mkdir $result_folder
    result_folder="$result_folder/${num_levels}_levels"
    mkdir $result_folder
    result_folder="$result_folder/$heuristic_name"
    mkdir $result_folder
    result_folder="$result_folder/${num_cores}_cores"
    mkdir $result_folder
    
    result_file="$result_folder/rg${i}_$sched_name"
    
    
    echo ./CPP_code/list_scheduler.out $dag_file $result_file $source_lgr_base$source_lgr -t $num_cores -l $num_levels $options
    ./CPP_code/list_scheduler.out $dag_file $result_file $source_lgr_base$source_lgr -t $num_cores -l $num_levels $options
    
done