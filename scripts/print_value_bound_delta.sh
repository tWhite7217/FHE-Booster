#!/bin/bash

# count = 0
# average = 0

for i in $(seq $2 $3)
do
    
    value=$(grep -a "Objective value" results/random_graph$i/rg${i}_$1.lgr | sed 's/\./ /g' | awk '{print $3}')
    bound=$(grep -a "Objective bound" results/random_graph$i/rg${i}_$1.lgr | sed 's/\./ /g' | awk '{print $3}')
    delta=$((value-bound))
    if (( delta > 0 )); then
        ./run_lingo_solver_on_multiple_graphs.sh $1 $i $i 1
        # count=$((count+1))
        # average=$((average+delta))
    fi
    echo $delta
    
done

# echo $count
# echo $((average/count))