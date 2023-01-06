#!/bin/bash

make solution_validator

for i in $(seq $2 $3)
do
    
    echo "$i:"
    ./CPP_code/solution_validator DDGs/random_graph$i/random_graph$i.txt results/random_graph$i/rg${i}_$1.lgr
    
done