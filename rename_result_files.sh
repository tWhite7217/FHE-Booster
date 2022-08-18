#!/bin/bash

for i in $(seq $1 $2)
do
    
    mv results/random_graph${i}/rg${i}_$3.lgr results/random_graph${i}/rg${i}_$4.lgr
    
done