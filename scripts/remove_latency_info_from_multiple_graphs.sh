#!/bin/bash

#Usage: <script_name> <first_graph_num> <last_graph_num>

first_graph_num=$1
last_graph_num=$2

for i in $(seq $first_graph_num $last_graph_num)
do
    tail -n +5 DAGs/random_graph${i}/random_graph${i}.txt > DAGs/random_graph${i}/random_graph${i}_no_latency.txt
    mv DAGs/random_graph${i}/random_graph${i}_no_latency.txt DAGs/random_graph${i}/random_graph${i}.txt
done