#!/bin/bash

first_graph_num=$1
last_graph_num=$2
levels=$3
heuristic_name=$4
num_cores=$5
sched_name=$6

for i in $(seq $first_graph_num $last_graph_num)
do
    
    cat results/random_graph$i/${levels}/$heuristic_name/${num_cores}_cores/rg${i}_${sched_name}.log
    
done