#!/bin/bash

make random_graph_generator.out
make txt_to_vcg.out
make input_file_processor.out

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
        
        mkdir DAGs/random_graph$graph_num
        ./CPP_code/random_graph_generator.out $num_operations $5 $6 $7 $8 $9 DAGs/random_graph$graph_num/random_graph$graph_num
        ./CPP_code/txt_to_vcg.out DAGs/random_graph$graph_num/random_graph$graph_num.txt DAGs/random_graph$graph_num/random_graph$graph_num.vcg
        ./CPP_code/input_file_processor.out DAGs/random_graph$graph_num/random_graph$graph_num.txt DAGs/random_graph$graph_num/random_graph$graph_num.LDT False
        ./CPP_code/input_file_processor.out DAGs/random_graph$graph_num/random_graph$graph_num.txt DAGs/random_graph$graph_num/random_graph${graph_num}_selective.LDT True
        sleep 1
        
    done
done