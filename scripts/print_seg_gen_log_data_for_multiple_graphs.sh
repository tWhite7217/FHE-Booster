#!/bin/bash

first_graph_num=$1
last_graph_num=$2
levels=$3

for i in $(seq $first_graph_num $last_graph_num)
do
    
    cat DAGs/random_graph$i/${levels}/bootstrap_segments.log
    cat DAGs/random_graph$i/${levels}/bootstrap_segments.log
    wc -c <"$file"
    
done