#!/bin/bash

#Usage <script_name> <first_graph_num> <last_graph_num>

first_graph_num=$1
last_graph_num=$2

for i in $(seq $first_graph_num $last_graph_num)
do
    mv DAGs/random_graph$i/9_levels DAGs/random_graph$i/b9_i0
done