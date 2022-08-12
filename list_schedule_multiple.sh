#!/bin/bash

make list_scheduler

source_lgr_suffix="min_bootstrapping"

if [[ "$1" == "selective" ]]; then
    source_lgr_suffix="min_bootstrapping_selective"
fi

source_lgr="NULL"
source_lgr_suffix=""

if [[ "$5" != "heuristic" ]]; then
    if [[ "$1" == "selective" ]]; then
        source_lgr_suffix="min_bootstrapping_selective.lgr"
    else
        source_lgr_suffix="min_bootstrapping.lgr"
    fi
fi

result_file_suffix=""

if [[ $(($4)) > 1 && ("$1" == "limited" || "$1" == "selective") ]]; then
    result_file_suffix="_$4_cores"
fi

for i in $(seq $2 $3)
do
    
    if [[ "$5" != "heuristic" ]]; then
        source_lgr="results/random_graph$i/rg${i}_"
    fi
    ./CPP_code/list_scheduler DDGs/random_graph$i/random_graph$i.txt $source_lgr$source_lgr_suffix results/random_graph$i/rg${i}_list_$1${result_file_suffix}_$5.lgr $4
    
done