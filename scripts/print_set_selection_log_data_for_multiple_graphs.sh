#!/bin/bash

first_graph_num=$1
last_graph_num=$2
levels=$3
heuristic_name=$4

for i in $(seq $first_graph_num $last_graph_num)
do
    
    cat results/random_graph$i/${levels}/$heuristic_name/complete_bootstrap_set.lgr.log
    
done