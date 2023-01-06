#!/bin/bash

for i in $(seq $2 $3)
do
    
    grep -a "Objective value" results/random_graph$i/rg${i}_$1.lgr | sed 's/\./ /g' | awk '{print $3}'
    
done