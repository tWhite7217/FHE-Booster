#!/bin/bash

first_graph_num=$1
last_graph_num=$2
levels=$3

for i in $(seq $first_graph_num $last_graph_num)
do
    
    grep -a "Elapsed runtime seconds" results/random_graph$i/${levels}/min_bootstrapping/complete_bootstrap_set.lgr | awk '{print $4}'
    
done