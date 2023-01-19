#!/bin/bash

make limited_to_selective_converter.out

for i in $(seq $1 $2)
do
    
    echo ./CPP_code/limited_to_selective_converter.out DAGs/random_graph${i}/random_graph${i}.txt results/random_graph${i}/rg${i}_$3.lgr results/random_graph${i}/rg${i}_min_bootstrapping_$4_levels$5_converted.lgr $4
    ./CPP_code/limited_to_selective_converter.out DAGs/random_graph${i}/random_graph${i}.txt results/random_graph${i}/rg${i}_$3.lgr results/random_graph${i}/rg${i}_min_bootstrapping_$4_levels$5_converted.lgr $4
    
done