#!/bin/bash

make limited_to_selective_converter

for i in $(seq $1 $2)
do
    
    ./CPP_code/limited_to_selective_converter DDGs/random_graph${i}/random_graph${i}.txt results/random_graph${i}/rg${i}_min_bootstrapping.lgr results/random_graph${i}/rg${i}_min_bootstrapping_converted.lgr
    
done