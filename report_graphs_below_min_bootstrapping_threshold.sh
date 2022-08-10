#!/bin/bash

for i in $(seq $1 $2)
do
    
    min_num_bootstrapping_operations=$(grep -a "Objective value" results/random_graph$i/rg${i}_min_bootstrapping.lgr | sed 's/\./ /g' | awk '{print $3}')
    if (( min_num_bootstrapping_operations < $3 )); then
        echo "Graph $i requires $min_num_bootstrapping_operations bootstrapping operations"
    fi
    
done