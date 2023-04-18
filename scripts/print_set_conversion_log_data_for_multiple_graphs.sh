#!/bin/bash

first_graph_num=$1
last_graph_num=$2
levels=$3
heuristic_name=$4

for i in $(seq $first_graph_num $last_graph_num)
do
    
    # head -1 results/random_graph$i/${levels}/$heuristic_name/converted_bootstrap_set.lgr.log
    sed '2!d' results/random_graph$i/${levels}/$heuristic_name/converted_bootstrap_set.lgr.log
    
done