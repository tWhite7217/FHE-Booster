#!/bin/bash

for i in $(seq $1 $2)
do
    
    grep -a "Elapsed runtime seconds" results/random_graph$i/9_levels/bootstrap_sets/rg${i}_min_bootstrapping.lgr | awk '{print $4}'
    
done