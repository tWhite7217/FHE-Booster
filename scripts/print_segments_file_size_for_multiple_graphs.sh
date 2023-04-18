#!/bin/bash

first_graph_num=$1
last_graph_num=$2
levels=$3

for i in $(seq $first_graph_num $last_graph_num)
do
    
    echo $i
    file="DAGs/random_graph$i/${levels}/bootstrap_segments_selective.txt"
    wc -c <"$file"
    
done