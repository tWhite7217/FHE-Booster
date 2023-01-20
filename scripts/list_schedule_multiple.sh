#!/bin/bash

#Usage <script_name> <source_lgr> <first_graph_num> <last_graph_num> <num_cores> <num_levels> <heuristic_name> <num_paths_multiplier> <slack_multiplier> <urgency_multiplier> <sched_name>

make list_scheduler.out

source_lgr=$1
first_graph_num=$2
last_graph_num=$3
num_cores=$4
num_levels=$5
heuristic_name=$6
num_paths_multiplier=$7
slack_multiplier=$8
urgency_multiplier=$9
sched_name=${10}

source_lgr_base=""

for i in $(seq $first_graph_num $last_graph_num)
do
    
    dag_file="DAGs/random_graph$i/random_graph$i.txt"
    
    if [[ "$source_lgr" != "NULL" ]]; then
        source_lgr_base="results/random_graph$i/${num_levels}_levels/bootstrapping_sets/rg${i}_"
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
    
    
    echo ./CPP_code/list_scheduler.out $dag_file $source_lgr_base$source_lgr $result_file $num_cores $num_paths_multiplier $slack_multiplier $urgency_multiplier $num_levels
    ./CPP_code/list_scheduler.out $dag_file $source_lgr_base$source_lgr $result_file $num_cores $num_paths_multiplier $slack_multiplier $urgency_multiplier $num_levels
    
done