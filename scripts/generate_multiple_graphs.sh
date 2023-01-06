#!/bin/bash

make graph_generator

graph_range=$(($2-$1+1))

for i in $(seq $1 $2 $3)
do
    for j in $(seq 1 $2)
    do
        
        graph_num=$((i-1+j))
        op_offset=$((i/$2))
        num_operations=$(($4+op_offset))
        
        echo $graph_num
        echo $op_offset
        echo $num_operations
        
        mkdir DDGs/random_graph$graph_num
        ./CPP_code/graph_generator $num_operations $5 $6 DDGs/random_graph$graph_num/random_graph$graph_num
        ./CPP_code/input_file_processor DDGs/random_graph$graph_num/random_graph$graph_num.txt DDGs/random_graph$graph_num/random_graph$graph_num.LDT False
        ./CPP_code/input_file_processor DDGs/random_graph$graph_num/random_graph$graph_num.txt DDGs/random_graph$graph_num/random_graph${graph_num}_selective.LDT True
        sleep 1
        
    done
done