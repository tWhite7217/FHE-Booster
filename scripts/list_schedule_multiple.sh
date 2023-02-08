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

source_lgr_bases=()

IFS=',' read -ra source_lgrs <<< "$source_lgr"
IFS=',' read -ra sched_names <<< "$sched_name"
IFS=',' read -ra heuristic_names <<< "$heuristic_name"
IFS=',' read -ra num_cores_arr <<< "$num_cores"

echo ${heuristic_names[1]}

for i in $(seq $first_graph_num $last_graph_num)
do
    dag_file="DAGs/random_graph$i/random_graph$i.txt"

    $input_lgrs=""

    for ((j = 0; j < ${#source_lgrs[@]}; j++))
    do
        if [[ "${source_lgrs[j]}" != "NULL" ]]; then
            source_lgr_bases+="results/random_graph$i/${num_levels}_levels/bootstrap_sets/rg${i}_"
        fi
        input_lgrs="$input_lgrs${source_lgr_bases[j]}${source_lgrs[j]},"
    done

    input_lgrs=${input_lgrs::-1}

    result_folder_base="results/random_graph$i"
    mkdir $result_folder_base
    result_folder_base="$result_folder_base/${num_levels}_levels"
    mkdir $result_folder_base

    result_folders=()
    result_files=""
    for ((j = 0; j < ${#source_lgrs[@]}; j++))
    do
        echo ${heuristic_names[j]}
        echo $result_folder_base
        result_folders[j]="$result_folder_base/${heuristic_names[j]}"
        mkdir ${result_folders[j]}
        result_folders[j]="${result_folders[j]}/${num_cores_arr[j]}_cores"
        mkdir ${result_folders[j]}
        result_files="$result_files${result_folders[j]}/rg${i}_${sched_names[j]},"
    done

    result_files=${result_files::-1}

    echo ./CPP_code/list_scheduler.out $dag_file $result_files -i $input_lgrs -t $num_cores -l $num_levels $options
    ./CPP_code/list_scheduler.out $dag_file $result_files -i $input_lgrs -t $num_cores -l $num_levels $options

done