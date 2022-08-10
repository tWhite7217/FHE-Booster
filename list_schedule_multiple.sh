#!/bin/bash

make list_scheduler

source_lgr_suffix="min_bootstrapping"

if [[ "$1" == "selective" ]]; then
    source_lgr_suffix="min_bootstrapping_selective"
fi

for i in $(seq $2 $3)
do
    
    ./CPP_code/list_scheduler DDGs/random_graph$i/random_graph$i.txt results/random_graph$i/rg${i}_$source_lgr_suffix.lgr results/random_graph$i/rg${i}_list_$1.lgr $4
    
done