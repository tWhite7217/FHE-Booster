#!/bin/bash

first_graph_num=$1
last_graph_num=$2
levels=$3

for i in $(seq $first_graph_num $last_graph_num)
do
    ./scripts/generate_both_LDT_variants.sh random_graph$i $levels
done