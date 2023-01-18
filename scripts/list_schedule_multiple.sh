#!/bin/bash

make list_scheduler.out


source_lgr="NULL"
source_lgr_suffix=""

if [[ "$6" != "heuristic" ]]; then
    if [[ "$1" == "limited" || "$1" == "unlimited" ]]; then
        source_lgr_suffix="min_bootstrapping_$8_levels.lgr"
    else
        source_lgr_suffix="min_bootstrapping_$8_levels_$1.lgr"
    fi
fi

result_file_suffix=""

if [[ $(($4)) > 1 && ("$1" != "unlimited") ]]; then
    result_file_suffix="_$4_cores"
fi

for i in $(seq $2 $3)
do
    
    if [[ "$6" != "heuristic" ]]; then
        source_lgr="results/random_graph$i/rg${i}_"
    fi
    
    echo ./CPP_code/list_scheduler.out DAGs/random_graph$i/random_graph$i.txt $source_lgr$source_lgr_suffix results/random_graph$i/rg${i}_$8_levels_list_$5${result_file_suffix} $4 $7
    ./CPP_code/list_scheduler.out DAGs/random_graph$i/random_graph$i.txt $source_lgr$source_lgr_suffix results/random_graph$i/rg${i}_$8_levels_list_$5${result_file_suffix} $4 $7
    
done